#include "depth_densify.h"

namespace tlz {

void depth_densify(const std::vector<Eigen_vec3>& samples, cv::Mat_<real>& out, cv::Mat_<uchar>& out_mask) {
	out.setTo(0.0);
	out_mask.setTo(0);
}

}
