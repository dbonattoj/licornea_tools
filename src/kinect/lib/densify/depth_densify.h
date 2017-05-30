#ifndef LICORNEA_KINECT_DEPTH_DENSIFY_H_
#define LICORNEA_KINECT_DEPTH_DENSIFY_H_

#include "../../../lib/opencv.h"
#include "../../../lib/common.h"
#include "../ir_to_color_sample.h"
#include <vector>
#include <memory>
#include <string>

namespace tlz {
	
class depth_densify_base {
public:
	using sample = ir_to_color_sample<real>;

	virtual ~depth_densify_base() = default;

	virtual void densify(const std::vector<sample>& samples, cv::Mat_<real>& out, cv::Mat_<uchar>& out_mask) = 0;
};

std::unique_ptr<depth_densify_base> make_depth_densify(const std::string& method);

}

#endif
