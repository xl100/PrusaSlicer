#ifndef SLICER_MARCHINGSQUARES_HPP
#define SLICER_MARCHINGSQUARES_HPP

#include "libslic3r/ExPolygon.hpp"
#include <type_traits>
#include <array>
#include <vector>

namespace Slic3r {

namespace marchsq {

template<class T> struct remove_cvref
{
    using type = std::remove_cv_t<std::remove_reference_t<T>>;
};

template<class T> using remove_cvref_t = typename remove_cvref<T>::type;

template<class T> struct _RasterTraits {
    using ValueType = typename T::ValueType;
    
    static ValueType val(const T &raster, size_t x, size_t y);
    static size_t rows(const T &raster);
    static size_t cols(const T &raster);
};

template<class T> using RasterTraits = _RasterTraits<remove_cvref_t<T>>;
template<class T> using TRasterValue = typename RasterTraits<T>::ValueType;

template<class T> TRasterValue<T> isoval(const T &raster, size_t x, size_t y)
{
    return RasterTraits<T>::get(raster, x, y);
}

template<class T> size_t rows(const T &raster)
{
    return RasterTraits<T>::rows(raster);
}

template<class T> size_t cols(const T &raster)
{
    return RasterTraits<T>::cols(raster);
}

template<class T> struct Tile {
    size_t row, col;
    T topleft, topright, bottomright, bottomleft;
};

} // namespace marchsq

template<class Raster>
ExPolygons marching_squares(const Raster &raster,
                            size_t        res_rows,
                            size_t        res_cols,
                            marchsq::TRasterValue<Raster> refval)
{
    using namespace marchsq;
    using IsoVal = TRasterValue<Raster>;
    
    size_t tilerows = rows(raster) / res_rows;
    size_t tilecols = rows(raster) / res_cols;
    std::vector< Tile<IsoVal> > tiles(tilerows * tilecols);
    
    for (size_t r = 0; r < tilerows; ++r)
        for(size_t c = 0; c < tilecols; ++c) {
            auto &t = tiles[r * tilecols + c];
            t.row = r;
            t.col = c;
            size_t r_tl = r * res_rows;
            size_t r_tr = r_tl;
            size_t c_tl = c * res_cols;
            size_t c_tr = c_tl + res_cols - 1;
            
            size_t r_bl = r_tl + res_rows - 1;
            size_t r_br = r_bl;
            size_t c_bl = c_tl;
            size_t c_br = c_tr;
            
            t.topleft = isoval(raster, r_tl, c_tl);
            t.topright = isoval(raster, r_tr, c_tr);
            t.bottomleft = isoval(raster, r_bl, c_bl);
            t.bottomright = isoval(raster, r_br, c_br);
        }
    
    
    
    return {};
}

} // namespace Slic3r

#endif // SLICER_MARCHINGSQUARES_HPP
