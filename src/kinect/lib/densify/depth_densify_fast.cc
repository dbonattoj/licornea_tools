#include "depth_densify_fast.h"
#include "../common.h"
#include <vector>
#include <algorithm>

namespace tlz {


void depth_densify_fast::densify(const std::vector<sample>& samples, cv::Mat_<real>& out, cv::Mat_<uchar>& out_mask) {
	const real scaledown = 0.3;
	cv::Size scaled_size(scaledown * texture_width, scaledown * texture_height);
	
	cv::Mat_<real> scaled_out(scaled_size);
	scaled_out.setTo(0.0);

	for(const sample& samp : samples) {
		int scaled_x = scaledown * samp.color_coordinates[0];
		int scaled_y = scaledown * samp.color_coordinates[1];
		if(scaled_x < 0 || scaled_x >= scaled_size.width || scaled_y < 0 || scaled_y >= scaled_size.height) continue;
		
		real new_d = samp.color_depth;
		real& d = scaled_out(scaled_y, scaled_x);
		if(d == 0.0 || new_d < d) d = new_d;
	}
	
	cv::resize(scaled_out, out, cv::Size(texture_width, texture_height), 0.0, 0.0, cv::INTER_NEAREST);
	out_mask = (out != 0.0);
}

}
