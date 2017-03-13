#ifndef LICORNEA_KINECT_TEXTURE_IO_H_
#define LICORNEA_KINECT_TEXTURE_IO_H_

#include "common.h"
#include "../../lib/opencv.h"

namespace tlz {

cv::Mat_<cv::Vec3b> load_texture(const char* filename);
void save_texture(const char* filename, const cv::Mat_<cv::Vec3b>&);

}

#endif
