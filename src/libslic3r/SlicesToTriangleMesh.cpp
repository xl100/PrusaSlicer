
#include "SlicesToTriangleMesh.hpp"

#include "libslic3r/TriangulateWall.hpp"
#include "libslic3r/SLA/Contour3D.hpp"
#include "libslic3r/ClipperUtils.hpp"
#include "libslic3r/Tesselate.hpp"

#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>

namespace Slic3r {

inline sla::Contour3D walls(const Polygon &lower,
                            const Polygon &upper,
                            double         lower_z_mm,
                            double         upper_z_mm)
{
    Wall w = triangulate_wall(lower, upper, lower_z_mm, upper_z_mm);
    
    sla::Contour3D ret;
    ret.points = std::move(w.first);
    ret.faces3 = std::move(w.second);
    
    return ret;
}

// Same as walls() but with identical higher and lower polygons.
sla::Contour3D inline straight_walls(const Polygon &plate,
                                     double         lo_z,
                                     double         hi_z)
{
    return walls(plate, plate, lo_z, hi_z);
}

sla::Contour3D inline straight_walls(const ExPolygon &plate,
                                     double           lo_z,
                                     double           hi_z)
{
    sla::Contour3D ret;
    ret.merge(straight_walls(plate.contour, lo_z, hi_z));
    for (auto &h : plate.holes) ret.merge(straight_walls(h, lo_z, hi_z));
    return ret;
}

sla::Contour3D inline straight_walls(const ExPolygons &slice,
                                     double            lo_z,
                                     double            hi_z)
{
    sla::Contour3D ret;
    for (const ExPolygon &poly : slice)
        ret.merge(straight_walls(poly, lo_z, hi_z));
    
    return ret;
}

sla::Contour3D slices_to_triangle_mesh(TriangleMesh &mesh,
                             const std::vector<ExPolygons> &slices,
                             const std::vector<double> &grid)
{
    assert(slices.size() == grid.size());

    using Layers = std::vector<sla::Contour3D>;
    std::vector<sla::Contour3D> layers(slices.size());
    size_t len = slices.size() - 1;

    tbb::parallel_for(size_t(0), len, [&slices, &layers, &grid](size_t i) {
        const ExPolygons &upper = slices[i + 1];
        const ExPolygons &lower = slices[i];

        ExPolygons dff1 = diff_ex(lower, upper);
        ExPolygons dff2 = diff_ex(upper, lower);
        layers[i].merge(triangulate_expolygons_3d(dff1, grid[i], NORMALS_UP));
        layers[i].merge(triangulate_expolygons_3d(dff2, grid[i], NORMALS_DOWN));
        layers[i].merge(straight_walls(upper, grid[i], grid[i + 1]));
    });

    return tbb::parallel_reduce(
        tbb::blocked_range(layers.begin(), layers.end()),
        sla::Contour3D{},
        [](const tbb::blocked_range<Layers::iterator>& r, sla::Contour3D init) {
            for(auto it = r.begin(); it != r.end(); ++it ) init.merge(*it);
            return init;
        },
        []( const sla::Contour3D &a, const sla::Contour3D &b ) {
            sla::Contour3D res{a}; res.merge(b); return res;
        });
}

void slices_to_triangle_mesh(TriangleMesh &                 mesh,
                             const std::vector<ExPolygons> &slices,
                             double                         zmin,
                             double                         lh,
                             double                         ilh)
{

    std::vector<sla::Contour3D> wall_meshes(slices.size());
    std::vector<double> grid(slices.size(), zmin + ilh);

    for (size_t i = 1; i < grid.size(); ++i) grid[i] = grid[i - 1] + lh;

    mesh.merge(sla::to_triangle_mesh(slices_to_triangle_mesh(mesh, slices, grid)));
    mesh.repair(true);
    mesh.reset_repair_stats();

//    sla::Contour3D lower;
//    lower.merge(triangulate_expolygons_3d(slices.front(), zmin, NORMALS_DOWN));
//    lower.merge(straight_walls(slices.front(), zmin, zmin + ilh));

//    sla::Contour3D cntr3d;
//    double h = zmin;
    
//    auto it = slices.begin(), xt = std::next(it);
//    cntr3d.merge(triangulate_expolygons_3d(*it, h, NORMALS_DOWN));
//    cntr3d.merge(straight_walls(*it, h, h + ilh));
//    h += ilh;
//    while (xt != slices.end()) {
//        ExPolygons dff1 = diff_ex(*it, *xt);
//        ExPolygons dff2 = diff_ex(*xt, *it);
//        cntr3d.merge(triangulate_expolygons_3d(dff1, h, NORMALS_UP));
//        cntr3d.merge(triangulate_expolygons_3d(dff2, h, NORMALS_DOWN));
//        cntr3d.merge(straight_walls(*xt, h, h + lh));
//        h += lh;
//        ++it; ++xt;
//    }
    
//    cntr3d.merge(triangulate_expolygons_3d(*it, h, NORMALS_UP));


//    mesh.merge(sla::to_triangle_mesh(cntr3d));
//    mesh.repair(true);
//    mesh.reset_repair_stats();
}

} // namespace Slic3r
