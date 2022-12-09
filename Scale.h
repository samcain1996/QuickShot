#pragma once

#include <Eigen/Dense>
#include "TypesAndDefs.h"

// X and Y positions of a pixel
using Coordinate = std::pair<int, int>;

using Pixel = std::span<MyByte>;
using ConstPixel = std::span<const MyByte>;

using Thing = std::array<MyByte, 4>;

using PixelList = std::vector<MyByte>;
using ConstPixelList = std::vector<ConstPixel>;

using Eigen::Matrix4d;
using Eigen::Matrix;

enum class Neighbor {
    TopLeft = 0, TopRight = 1,
    BottomLeft = 2, BottomRight = 3
};

constexpr const int BYTES_PER_PIXEL = NUM_COLOR_CHANNELS;

using PixelAndPos = std::pair<ConstPixel, Coordinate>;
using Neighbors = std::array<PixelAndPos, BYTES_PER_PIXEL>;
/*----------Pixel Functions----------*/

// Convert 1-D index to 2-D coordinate
static const size_t CoordinateToIndex(const Resolution& res, const Coordinate& coord);
// Convert 2-D coordinate to 1-D index
static const Coordinate IndexToCoordinate(const Resolution& res, const size_t index);

// Convert between index of pixels and index of individual bytes
static const size_t ConvertIndex(const size_t index, const bool toAbsoluteIndex = true);

// Returns pixel at index of data
static Pixel GetPixel(PixelData& data, const size_t index, const bool isAbsoluteIndex = true);
// Returns a const pixel at index of data
static ConstPixel GetPixel(const PixelData& data, const size_t index, const bool isAbsoluteIndex = true);

// Assign 1 pixel's values to another
static void AssignPixel(Pixel& assignee, const ConstPixel& other);

static void SubtractPixel(Pixel& subFrom, const ConstPixel& sub);
static Thing SubtractPixel(const ConstPixel& subFrom, const ConstPixel& sub);


/*-----------------------------------*/


// Scale between two images in x and y directions ( new / old )
struct ScaleRatio {
    double xRatio = 1;
    double yRatio = 1;

    ScaleRatio(const double x, const double y) : xRatio(x), yRatio(y) {}
    ScaleRatio(const std::pair<double, double>& ratio) : xRatio(ratio.first), yRatio(ratio.second) {}
    ScaleRatio(const Resolution& ratio) : xRatio(ratio.width), yRatio(ratio.height) {}
};


class Scaler {

public:

    // Supported scaling methods
    enum class ScaleMethod {
        NearestNeighbor,
        Bilinear,
        Bicubic,
        Lanczos   // Not implemented
    };

    // Default Scaling Method
    static inline ScaleMethod method = ScaleMethod::NearestNeighbor; 

    static PixelData Scale(const PixelData& sourceImage,
        const Resolution& sourceResolution, const Resolution& destResolution);

    static PixelData Scale(const PixelData& sourceImage,
        const Resolution& sourceResolution, const ScaleRatio& scaleRatio);

    static PixelData Scale(const PixelData& sourceImage,
        const Resolution& sourceResolution, const Uint32 scalingFactor);

private:

    // Class shouldn't be instantiated, it is static
    Scaler() = delete;
    ~Scaler() = delete;

    // Get the ratio in the x and y directions between dest and source images
    static ScaleRatio GetScaleRatio(const Resolution& source, const Resolution& dest);

    static inline Neighbors GetNeighbors(const double x, const double y, const PixelData& source,
        const Resolution& src);

    /* ----- Scaling Functions ----- */

    // Upscale using nearest neighbor ( blockiest results )
    static PixelData NearestNeighbor(const PixelData& source, const Resolution& src, const Resolution& dest);

    // Upscale by linearly interpolating pixel values ( blurry )
    static PixelData Bilinear(const PixelData& source, const Resolution& src, const Resolution& dest);

    // TODO: Implement other scaling methods
    static PixelData Bicubic(const PixelData& source, const Resolution& src, const Resolution& dest);
    static PixelData Lanczos(const PixelData& source, const Resolution& src, const Resolution& dest);


};