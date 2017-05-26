#ifndef LICORNEA_COLOR_H_
#define LICORNEA_COLOR_H_

#include <cstdint>
#include "common.h"
#include "opencv.h"

namespace tlz {

/// RGB color, 8 bit.
struct rgb_color {
	rgb_color() = default;
	rgb_color(std::uint8_t nr, std::uint8_t ng, std::uint8_t nb) :
		r(nr), g(ng), b(nb) { }
	
	rgb_color(const rgb_color&) = default;
	rgb_color& operator=(const rgb_color&) = default;
	
	std::uint8_t r; // red
	std::uint8_t g; // green
	std::uint8_t b; // blue
	
	const static rgb_color black;
	const static rgb_color white;
};

bool operator==(const rgb_color& a, const rgb_color& b);
bool operator!=(const rgb_color& a, const rgb_color& b);


/// YCbCr color, 8 bit.
struct ycbcr_color {
	ycbcr_color() = default;
	ycbcr_color(std::uint8_t ny, std::uint8_t ncr, std::uint8_t ncb) :
		y(ny), cr(ncr), cb(ncb) { }
	
	ycbcr_color(const ycbcr_color&) = default;
	ycbcr_color& operator=(const ycbcr_color&) = default;

	std::uint8_t y;  // luma, Y'
	std::uint8_t cr; // chroma-red, U
	std::uint8_t cb; // chroma-blue, V
};

bool operator==(const ycbcr_color& a, const ycbcr_color& b);
bool operator!=(const ycbcr_color& a, const ycbcr_color& b);



/// Color conversion, specialized for different color formats.
template<typename Output, typename Input>
Output color_convert(const Input&);

template<> rgb_color color_convert(const ycbcr_color&);
template<> ycbcr_color color_convert(const rgb_color&);


/// Color blend.
rgb_color color_blend(const rgb_color& a, const rgb_color& b);
rgb_color color_blend(const rgb_color& a, real a_weight, const rgb_color& b, real b_weight);

}


namespace cv { // in OpenCV namespace
	template<>
	class DataType<::tlz::rgb_color> {
	public:
		using value_type = ::tlz::rgb_color;
		using work_type = int;
		using channel_type = uchar;
		enum {
			generic_type = 0,
			depth = DataDepth<channel_type>::value,
			channels = 3,
			fmt = ((channels - 1)<<8) + DataDepth<channel_type>::fmt,
			type = CV_MAKETYPE(depth, channels)
		};
		using vec_type = Vec<channel_type, channels>;
	};
	
	template<>
	class DataType<::tlz::ycbcr_color> {
	public:
		using value_type = ::tlz::ycbcr_color;
		using work_type = int;
		using channel_type = uchar;
		enum {
			generic_type = 0,
			depth = DataDepth<channel_type>::fmt,
			channels = 3,
			fmt = ((channels - 1)<<8) + DataDepth<channel_type>::fmt,
			type = CV_MAKETYPE(depth, channels)
		};
		using vec_type = Vec<channel_type, channels>;
	};
}



#endif
