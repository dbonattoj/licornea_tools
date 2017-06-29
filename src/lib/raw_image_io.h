#ifndef LICORNEA_RAW_IMAGE_IO_H_
#define LICORNEA_RAW_IMAGE_IO_H_

#include "common.h"

namespace tlz {

enum class raw_image_format {
	ycbcr420,
	rgb_planar,
	rgb_interleaved
};

cv::Mat import_raw_color(const std::string& yuv_filename, int width, int height, raw_image_format);
cv::Mat import_raw_mono(const std::string& yuv_filename, int width, int height, int bit_depth = 8);

void export_raw_color(const cv::Mat& img, const std::string& yuv_filename, raw_image_format);
void export_raw_mono(const cv::Mat& img, const std::string& yuv_filename, int bit_depth = -1);

}

#endif
