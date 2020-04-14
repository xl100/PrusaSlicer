#include "SLAZipFileImport.hpp"

#include "libslic3r/SlicesToTriangleMesh.hpp"
#include "libslic3r/MarchingSquares.hpp"
#include "libslic3r/ClipperUtils.hpp"
#include "libslic3r/MTUtils.hpp"
#include "libslic3r/PrintConfig.hpp"
#include "libslic3r/SLA/RasterBase.hpp"

#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/mstream.h>
#include <wx/sstream.h>
#include <wx/image.h>

#include <tbb/parallel_for.h>

#include <boost/property_tree/ini_parser.hpp>

namespace marchsq {

// Specialize this struct to register a raster type for the Marching squares alg
template<> struct _RasterTraits<wxImage> {
    using Rst = wxImage;
    
    // The type of pixel cell in the raster
    using ValueType = uint8_t;
    
    // Value at a given position
    static uint8_t get(const Rst &rst, size_t row, size_t col) { return rst.GetRed(col, row); }
    
    // Number of rows and cols of the raster
    static size_t rows(const Rst &rst) { return rst.GetHeight(); }
    static size_t cols(const Rst &rst) { return rst.GetWidth(); }
};

} // namespace marchsq

namespace Slic3r {

ExPolygons rings_to_expolygons(const std::vector<marchsq::Ring> &rings,
                               double px_w, double px_h)
{
    ExPolygons polys; polys.reserve(rings.size());
    
    for (const marchsq::Ring &ring : rings) {
        Polygon poly; Points &pts = poly.points;
        pts.reserve(ring.size());
        
        for (const marchsq::Coord &crd : ring)
            pts.emplace_back(scaled(crd.c * px_w), scaled(crd.r * px_h));
        
        polys.emplace_back(poly);
    }
    
    // reverse the raster transformations
    return union_ex(polys);
}

template<class Fn> void foreach_vertex(ExPolygon &poly, Fn &&fn)
{
    for (auto &p : poly.contour.points) fn(p);
    for (auto &h : poly.holes)
        for (auto &p : h.points) fn(p);
}

void invert_raster_trafo(ExPolygons &                  expolys,
                         const sla::RasterBase::Trafo &trafo,
                         coord_t                       width,
                         coord_t                       height)
{
    for (auto &expoly : expolys) {
        if (trafo.mirror_y)
            foreach_vertex(expoly, [height](Point &p) {p.y() = height - p.y(); });
        
        if (trafo.mirror_x)
            foreach_vertex(expoly, [width](Point &p) {p.x() = width - p.x(); });
        
        expoly.translate(-trafo.center_x, -trafo.center_y);
        
        if (trafo.flipXY)
            foreach_vertex(expoly, [](Point &p) { std::swap(p.x(), p.y()); });
        
        if ((trafo.mirror_x + trafo.mirror_y + trafo.flipXY) % 2) {
            expoly.contour.reverse();
            for (auto &h : expoly.holes) h.reverse();
        }
    }
}

void extract_ini(const std::string &path,
              std::map<std::string, wxMemoryOutputStream> &archive,
              boost::property_tree::ptree &tree)
{
    auto it = archive.find(path);
    if (it != archive.end()) {
        wxString str;
        wxStringOutputStream oss{&str};
        wxMemoryInputStream inp{it->second};
        oss.Write(inp);
        std::stringstream iss(str.ToStdString());
        boost::property_tree::read_ini(iss, tree);
        archive.erase(it);
    } else {
        throw std::runtime_error(path + " is missing");
    }
}

TriangleMesh import_model_from_sla_zip(const wxString &zipfname, Point windowsize)
{
    // Ensure minimum window size for marching squares
    windowsize.x() = std::max(2, windowsize.x());
    windowsize.y() = std::max(2, windowsize.y());
    
    wxFileInputStream in(zipfname);
    wxZipInputStream zip(in, wxConvUTF8);
    
    std::map<std::string, wxMemoryOutputStream> files;
    
    while (auto entry = std::unique_ptr<wxZipEntry>(zip.GetNextEntry())) {
        auto fname = wxFileName(entry->GetName());
        wxString name_lo = fname.GetFullName().Lower();
        
        if (fname.IsDir() || name_lo.Contains("thumbnail")) continue;
        
        if (!zip.OpenEntry(*entry))
            throw std::runtime_error("Cannot read archive");
        
        wxMemoryOutputStream &stream = files[name_lo.ToStdString()];
        zip.Read(stream);
        if (!zip.LastRead()) std::cout << zip.GetLastError() << std::endl;
    }
    
    boost::property_tree::ptree profile_tree, config;
    extract_ini("prusaslicer.ini", files, profile_tree);
    extract_ini("config.ini", files, config);    
    
    DynamicPrintConfig profile;
    profile.load(profile_tree);
    
    auto jobdir = config.get<std::string>("jobDir");
    for (auto &c : jobdir) c = std::tolower(c);
    
    for (auto it = files.begin(); it != files.end();)
        if (it->first.find(jobdir) == std::string::npos ||
            wxFileName(it->first).GetExt().Lower() != "png")
            it = files.erase(it);
        else ++it;
    
    auto *opt_disp_cols = profile.option<ConfigOptionInt>("display_pixels_x");
    auto *opt_disp_rows = profile.option<ConfigOptionInt>("display_pixels_y");
    auto *opt_disp_w    = profile.option<ConfigOptionFloat>("display_width");
    auto *opt_disp_h    = profile.option<ConfigOptionFloat>("display_height");
    auto *opt_mirror_x  = profile.option<ConfigOptionBool>("display_mirror_x");
    auto *opt_mirror_y  = profile.option<ConfigOptionBool>("display_mirror_y");
    auto *opt_orient    = profile.option<ConfigOptionEnum<SLADisplayOrientation>>("display_orientation");

    if (!opt_disp_cols || !opt_disp_rows || !opt_disp_w || !opt_disp_h ||
        !opt_mirror_x || !opt_mirror_y || !opt_orient)
        throw std::runtime_error("Invalid SL1 file");
    
    struct RasterParams {
        sla::RasterBase::Trafo trafo;   // Raster transformations
        coord_t width, height;      // scaled raster dimensions (not resolution)
        double px_h, px_w;          // pixel dimesions
        marchsq::Coord win;         // marching squares window size
    } rstp;
        
    rstp.px_w = opt_disp_w->value / opt_disp_cols->value;
    rstp.px_h = opt_disp_h->value / opt_disp_rows->value;

    sla::RasterBase::Trafo trafo{opt_orient->value == sladoLandscape ?
                                     sla::RasterBase::roLandscape :
                                     sla::RasterBase::roPortrait,
                                 {opt_mirror_x->value, opt_mirror_y->value}};
    
    rstp.height = scaled(opt_disp_h->value);
    rstp.width  = scaled(opt_disp_w->value);
    
    std::vector<ExPolygons> slices(files.size());
    std::vector<std::reference_wrapper<wxMemoryOutputStream>> streams;
    streams.reserve(files.size());
    for (auto &item : files) streams.emplace_back(std::ref(item.second));
    
    rstp.win = {size_t(windowsize.x()), size_t(windowsize.y())};
    
    tbb::parallel_for(size_t(0), files.size(),
                      [&streams, &slices, &rstp](size_t i) {
                          
        wxMemoryOutputStream &imagedata = streams[i];
        wxMemoryInputStream stream{imagedata};
        wxImage img{stream};
        
        auto rings = marchsq::execute(img, 128, rstp.win);
        ExPolygons expolys = rings_to_expolygons(rings, rstp.px_w, rstp.px_h);
        
        // Invert the raster transformations indicated in the profile metadata
        invert_raster_trafo(expolys, rstp.trafo, rstp.width, rstp.height);
        
        slices[i] = std::move(expolys);
    });
    
    TriangleMesh out;
    if (!slices.empty()) {
        double lh  = profile.opt_float("layer_height");
        double ilh = profile.opt_float("initial_layer_height");
        out = slices_to_triangle_mesh(slices, 0, lh, ilh);
    }
    
    return out;
}

} // namespace Slic3r
