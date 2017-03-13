#ifndef LICORNEA_KINECT_DEPTH_DENSIFY_H_
#define LICORNEA_KINECT_DEPTH_DENSIFY_H_

#include "../../lib/eigen.h"
#include "../../lib/opencv.h"
#include "../../lib/common.h"
#include <vector>

namespace tlz {

void depth_densify(const std::vector<Eigen_vec3>& samples, cv::Mat_<real>& out, cv::Mat_<uchar>& out_mask);

}

#endif
