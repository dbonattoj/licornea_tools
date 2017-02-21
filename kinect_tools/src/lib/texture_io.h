#ifndef K3DLIC_TEXTURE_IO_H_
#define K3DLIC_TEXTURE_IO_H_

#include "common.h"
#include <opencv2/opencv.hpp>

cv::Mat_<cv::Vec3b> load_texture(const char* filename);
void save_texture(const char* filename, const cv::Mat_<cv::Vec3b>&);


#endif
