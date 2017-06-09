#include "depth_densify.h"
#include "depth_densify_mine.h"
#include "depth_densify_splat.h"
#include "depth_densify_fast.h"

namespace tlz {

void depth_densify_base::densify(const std::vector<sample>& samples, cv::Mat_<real>& out) {
	cv::Mat_<uchar> unused_mask;
	this->densify(samples, out, unused_mask);
}

std::unique_ptr<depth_densify_base> make_depth_densify(const std::string& method) {
	if(method == "mine") return std::make_unique<depth_densify_mine>();
	else if(method == "splat") return std::make_unique<depth_densify_splat>();
	else if(method == "fast") return std::make_unique<depth_densify_fast>();
	else throw std::invalid_argument("unknown depth densify method");
}


}
