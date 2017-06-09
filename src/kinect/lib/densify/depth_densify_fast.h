#ifndef LICORNEA_KINECT_DEPTH_DENSIFY_FAST_H_
#define LICORNEA_KINECT_DEPTH_DENSIFY_FAST_H_

#include "depth_densify.h"

namespace tlz {
	
class depth_densify_fast : public depth_densify_base {
public:
	void densify(const std::vector<sample>& samples, cv::Mat_<real>& out, cv::Mat_<uchar>& out_mask) override;
};

}

#endif

