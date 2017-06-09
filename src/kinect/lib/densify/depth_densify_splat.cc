#include "depth_densify_splat.h"
#include "../common.h"
#include <vector>
#include <algorithm>

namespace tlz {


void depth_densify_splat::densify(const std::vector<sample>& orig_samples, cv::Mat_<real>& out, cv::Mat_<uchar>& out_mask) {
	out = cv::Mat_<real>(texture_height, texture_width);
	out_mask = cv::Mat_<uchar>(texture_height, texture_width);

	std::vector<sample> samples = orig_samples;
	auto cmp = [](const sample& a, const sample& b) { return (a.color_depth > b.color_depth); };
	std::sort(samples.begin(), samples.end(), cmp);

	out.setTo(0.0);
	out_mask.setTo(0);
	
	int rad = 4;
	for(const sample& samp : samples) {
		int sx = samp.color_coordinates[0], sy = samp.color_coordinates[1];
		if(sx < 0 || sx >= texture_width || sy < 0 || sy >= texture_height) continue;
		
		real new_d = samp.color_depth;
		
		cv::circle(out, cv::Point(sx, sy), rad, new_d, -1);
		cv::circle(out_mask, cv::Point(sx, sy), rad, 0xff, -1);
	}
	
	std::cout << "done" << std::endl;
}

}
