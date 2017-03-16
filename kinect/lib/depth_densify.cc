#include "depth_densify.h"
#include "../../lib/nd.h"
#include "common.h"
#include <vector>
#include <stack>
#include <string>
#include <iostream>
#include <array>
#include <algorithm>
#include <random>

namespace tlz {
	
namespace {

		
}

void depth_densify(const std::vector<Eigen_vec3>& samples, cv::Mat_<real>& out, cv::Mat_<uchar>& out_mask) {
	out.setTo(0.0);
	out_mask.setTo(0);

	using sample_ptr = const Eigen_vec3*;
	int neighborhood_samples_max = 30;
	int max_dist = 6;
	ndarray<3, sample_ptr> neighborhood_samples(make_ndsize(texture_width, texture_height, neighborhood_samples_max));
	ndarray<2, int> neighborhood_samples_count(make_ndsize(texture_width, texture_height));
	std::fill(neighborhood_samples.begin(), neighborhood_samples.end(), nullptr);
	std::fill(neighborhood_samples_count.begin(), neighborhood_samples_count.end(), 0);
	
	std::default_random_engine random_gen;
    std::uniform_int_distribution<std::ptrdiff_t> random_distribution(0, neighborhood_samples_max - 1);
    
    int max_dist_sq = max_dist * max_dist;
    for(const Eigen_vec3& sample : samples) {
		int sx = sample[0], sy = sample[1];
		if(sx < 0 || sx >= texture_width || sy < 0 || sy >= texture_height) continue;
		
		for(int px = std::max(sx - max_dist, 0); px <= std::min(sx + max_dist, (int)texture_width-1); ++px)
		for(int py = std::max(sy - max_dist, 0); py <= std::min(sy + max_dist, (int)texture_height-1); ++py) {
			int off_x = sx - px, off_y = sy - py;
			if(off_x*off_x + off_y*off_y > max_dist_sq) continue;

			
			int& n_count = neighborhood_samples_count[px][py];
			ndarray_view<1, sample_ptr> n_samples = neighborhood_samples[px][py];
			if(n_count < neighborhood_samples_max) n_samples[n_count++] = &sample;
			else n_samples[random_distribution(random_gen)] = &sample;
		}
		
		out(sy, sx) = sample[2];
		out_mask(sy, sx) = 0xff;
	}

	for(int px = 0; px < texture_width; ++px)
	for(int py = 0; py < texture_height; ++py) {
		if(out_mask(py, px)) continue;
		
		const int& n_count = neighborhood_samples_count[px][py];
		if(n_count == 0) continue;
		
		ndarray_view<1, const sample_ptr> n_samples = neighborhood_samples[px][py];

		real depths_sum = 0;
		real depths_div = 0;
		int directions[2][2] = {0,0,0,0};
		
		for(int i = 0; i < n_count; ++i) {
			const Eigen_vec3& n_sample = *n_samples[i];
			real off_x = n_sample[0] - px;
			real off_y = n_sample[1] - py;
			real dist_sq = off_x*off_x + off_y*off_y;
			
			//if(off_y < 0) std::cout << off_y << std::endl;
			
			directions[off_x < 0.0][off_y < 0.0]++;
			
			real w = max_dist_sq - dist_sq;
			depths_sum += w*n_sample[2];
			depths_div += w;
		}
		
		//std::cout << directions[0][0] << "," << directions[0][1] << "," << directions[1][0] << "," << directions[1][1] << std::endl;		
		
		if(std::min({directions[0][0], directions[0][1], directions[1][0], directions[1][1]}) < 1) continue;
		
		out(py, px) = depths_sum / depths_div;
		out_mask(py, px) = 0xff;
	}
	
	out = 20.0*out;
}

void depth_densify_combine(const std::vector<depth_sample_set>& sample_sets, cv::Mat_<real>& out, cv::Mat_<uchar>& out_mask) {
	out.setTo(0.0);
	out_mask.setTo(0);

	
}

}
