#ifndef MARCHINGCUBES2D_HPP
#define MARCHINGCUBES2D_HPP

#include "libslic3r/ExPolygon.hpp"
#include <type_traits>

namespace Slic3r {

namespace marchsq {

template<class T> struct remove_cvref
{
    using type = std::remove_cv_t<std::remove_reference_t<T>>;
};

template<class T> using remove_cvref_t = typename remove_cvref<T>::type;

template<class T> struct _RasterTraits {
    using PixelType = typename T::PixelType;
    
    static PixelType get(const T &raster, size_t x, size_t y);
    static size_t rows(const T &raster);
    static size_t cols(const T &raster);
};

template<class T> using RasterTraits = _RasterTraits<remove_cvref_t<T>>;
template<class T> using TRasterPixel = typename RasterTraits<T>::PixelType;

template<class T> TRasterPixel<T> get(const T &raster, size_t x, size_t y)
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

template<class Raster>
ExPolygons marching_squares(const Raster &raster,
                            size_t        res_x,
                            size_t        res_y,
                            size_t        isoval)
{
    ExPolygons polygons;
    
    Polygon path;
    
    for (size_t x = 0; x < rows(raster); ++x)
        for(size_t y = 0; y < cols(raster); ++y) {
            
        }
            
    return {};
}

}
} // namespace Slic3r

#endif // MARCHINGCUBES2D_HPP
