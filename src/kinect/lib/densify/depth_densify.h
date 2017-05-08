#ifndef LICORNEA_KINECT_DEPTH_DENSIFY_H_
#define LICORNEA_KINECT_DEPTH_DENSIFY_H_

#include "../../../lib/opencv.h"
#include "../../../lib/common.h"
#include <vector>
#include <memory>
#include <string>

namespace tlz {
	
class depth_densify_base {
public:
	struct sample_set {
		std::vector<vec3> samples;
		real distance;
	};
	
	virtual ~depth_densify_base() = default;

	virtual void densify(const std::vector<vec3>& samples, cv::Mat_<real>& out, cv::Mat_<uchar>& out_mask) = 0;
	//virtual void densify_merge(const std::vector<sample_set>& sample_sets, cv::Mat_<real>& out, cv::Mat_<uchar>& out_mask) = 0;
};

std::unique_ptr<depth_densify_base> make_depth_densify(const std::string& method);

}

#endif
