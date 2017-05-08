#include "depth_densify_splat.h"
#include "../common.h"
#include <vector>
#include <algorithm>

namespace tlz {


void depth_densify_splat::densify(const std::vector<vec3>& orig_samples, cv::Mat_<real>& out, cv::Mat_<uchar>& out_mask) {
	std::vector<vec3> samples = orig_samples;
	auto cmp = [](const vec3& a, const vec3& b) { return (a[2] > b[2]); };
	std::sort(samples.begin(), samples.end(), cmp);

	out.setTo(0.0);
	out_mask.setTo(0);
	
	int rad = 4;
	for(const vec3& sample : samples) {
		int sx = sample[0], sy = sample[1];
		if(sx < 0 || sx >= texture_width || sy < 0 || sy >= texture_height) continue;
		
		real new_d = sample[2];
		
		cv::circle(out, cv::Point(sx, sy), rad, new_d, -1);
		cv::circle(out_mask, cv::Point(sx, sy), rad, 0xff, -1);
	}	
}

}
