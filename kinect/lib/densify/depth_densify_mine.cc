#include "depth_densify_mine.h"
#include "../common.h"
#include <vector>
#include <algorithm>

namespace tlz {


void depth_densify_mine::densify(const std::vector<Eigen_vec3>& orig_samples, cv::Mat_<real>& out, cv::Mat_<uchar>& out_mask) {
	std::vector<Eigen_vec3> samples = orig_samples;
	auto cmp = [](const Eigen_vec3& a, const Eigen_vec3& b) { return (a[2] > b[2]); };
	std::sort(samples.begin(), samples.end(), cmp);
	
	cv::Mat_<uchar> sparse_mask(texture_height, texture_width);
	cv::Mat_<real> sparse(texture_height, texture_width);
	sparse_mask.setTo(0);
	
	int shadow_width = 3;
	real shadow_min_depth_diff = 100.0;
	for(const Eigen_vec3& sample : samples) {
		int sx = sample[0], sy = sample[1];
		if(sx < 0 || sx >= texture_width || sy < 0 || sy >= texture_height) continue;
		
		real new_d = sample[2];
	
		// cast shadow
		int min_x = std::max(sx - shadow_width, 0), max_x = std::min(sx + shadow_width, (int)texture_width-1);
		int min_y = std::max(sy - shadow_width, 0), max_y = std::min(sy + shadow_width, (int)texture_height-1);
		for(int x = min_x; x <= max_x; ++x) if(x != sx)
		for(int y = min_y; y <= max_y; ++y) if(y != sy) {
			if(! sparse_mask(y, x)) continue;
			if(sparse(y, x) - new_d > shadow_min_depth_diff) sparse_mask(y, x) = 0;
		}

		sparse(sy, sx) = new_d;
		sparse_mask(sy, sx) = 0xff;
	}	
	
	out.setTo(0.0);
	out_mask.setTo(0);

	/*
	out=sparse;
	out.setTo(0, sparse_mask==0);
	out_mask=sparse_mask;
	return;
	*/
	
	int rad = 6;
	int rad_sq = rad*rad;

	for(int px = 0; px < texture_width; ++px)
	for(int py = 0; py < texture_height; ++py) {
		uchar& mask = out_mask(py, px);
		real& d = out(py, px);

		if(sparse_mask(py, px)) {
			d = sparse(py, px);
			mask = 0xff;
			//continue;
		}
			
		int min_dist = rad*rad;
		real min_dist_d = 0.0;
		
		real max_d = 0.0;
		
		int samples_count = 0;
		
		int accept_dist = 4;
		
		int min_x = std::max(px - rad, 0), max_x = std::min(px + rad, (int)texture_width-1);
		int min_y = std::max(py - rad, 0), max_y = std::min(py + rad, (int)texture_height-1);
		for(int sx = min_x; sx <= max_x; ++sx) if(sx != px)
		for(int sy = min_y; sy <= max_y; ++sy) if(sy != py) {
			if(! sparse_mask(sy, sx)) continue;
			real sd = sparse(sy, sx);
			
			int off_x = sx - px, off_y = sy - py;
			
			int dist = std::max(std::abs(off_x), std::abs(off_y));
			int euc_sq_dist = off_x*off_x + off_y*off_y;


			if(euc_sq_dist > rad_sq) continue;
			
			samples_count++;
			
			//if(sx % 100 != 0 || sy % 100 != 0) continue;
			
			if(dist < min_dist) {
				min_dist = dist;
				min_dist_d = sd;
			}
			
			if(sd > max_d) {
				max_d = sd;
			}
		}
		
		if(samples_count > 0) {
			mask = 0xff;
			if(min_dist < accept_dist) d = min_dist_d;
			else d = max_d;
		}	
	}
	
	cv::medianBlur(out, out, 3);
}

}
