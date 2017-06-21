#ifndef LICORNEA_IMAGE_IO_H_
#define LICORNEA_IMAGE_IO_H_

#include "common.h"
#include "opencv.h"
#include <string>

namespace tlz {

cv::Mat_<cv::Vec3b> load_texture(const std::string& filename);
void save_texture(const std::string& filename, const cv::Mat_<cv::Vec3b>&);

cv::Mat_<ushort> load_ir(const std::string& filename);
void save_ir(const std::string& filename, const cv::Mat_<ushort>&);

cv::Mat_<ushort> load_depth(const std::string& filename);
void save_depth(const std::string& filename, const cv::Mat_<ushort>&);

}

#endif
