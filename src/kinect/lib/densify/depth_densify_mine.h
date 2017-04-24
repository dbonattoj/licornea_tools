#ifndef LICORNEA_KINECT_DEPTH_DENSIFY_MINE_H_
#define LICORNEA_KINECT_DEPTH_DENSIFY_MINE_H_

#include "depth_densify.h"

namespace tlz {
	
class depth_densify_mine : public depth_densify_base {
public:
	void densify(const std::vector<Eigen_vec3>& samples, cv::Mat_<real>& out, cv::Mat_<uchar>& out_mask) override;
	//virtual void densify_merge(const std::vector<sample_set>& sample_sets, cv::Mat_<real>& out, cv::Mat_<uchar>& out_mask) = 0;
};

}

#endif

