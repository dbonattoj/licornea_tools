#ifndef LICORNEA_KINECT_DEPTH_IO_H_
#define LICORNEA_KINECT_DEPTH_IO_H_

#include "common.h"
#include "../../lib/opencv.h"

namespace tlz {

cv::Mat_<ushort> load_depth(const char* filename, bool any_size = false);
void save_depth(const char* filename, const cv::Mat_<ushort>&);

}

#endif
