#ifndef K3DLIC_DEPTH_IO_H_
#define K3DLIB_DEPTH_IO_H_

#include "common.h"
#include <opencv2/opencv.hpp>

cv::Mat_<ushort> load_depth(const char* filename);
void save_depth(const char* filename, const cv::Mat_<ushort>&);

#endif
