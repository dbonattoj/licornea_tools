#include <iostream>
#include <utility>
#include <vector>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <set>
#include <map>
#include "lib/image_correspondence.h"
#include "../lib/args.h"
#include "../lib/misc.h"
#include "../lib/intrinsics.h"
#include "../lib/image_io.h"
#include "../lib/json.h"
#include "../lib/opencv.h"

using namespace tlz;

constexpr bool verbose = true;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "image_correspondences.json intrinsics.json R.json out_straight_depths.json");
	image_correspondences cors = image_correspondences_arg();
	intrinsics intr = intrinsics_arg();
	mat33 R = decode_mat(json_arg());
	std::string out_feature_depths = out_filename_arg();

	image_correspondences undist_cors = undistort(cors, intr);

	std::map<std::string, std::map<view_index, real>> feature_view_straight_depths;
	
	
	std::cout << "computing feature straight depths" << std::endl;
	for(const auto& kv : undist_cors.features) {
		const std::string& feature_name = kv.first;
		const image_correspondence_feature& feature = kv.second;
		for(const auto& kv2 : feature.points) {
			const view_index& view_idx = kv2.first;
			const feature_point& fpoint = kv2.second;
			
			vec2 i_measured_undist = fpoint.position;
			real d_measured = fpoint.depth;
		
			vec3 i_measured_h(i_measured_undist[0], i_measured_undist[1], 1.0);
			i_measured_h *= d_measured;
			
			vec3 v_measured = intr.K_inv * i_measured_h;
			vec3 v_straight = R.t() * v_measured;
			real d_straight = v_straight[2];
			
			feature_view_straight_depths[feature_name][view_idx] = d_straight;
		}
		
	}
	

	std::cout << "aggregating feature straight depths" << std::endl;
	std::map<std::string, real> avg_straight_depths;
	real avg_stddev = 0.0;
	for(const auto& kv : feature_view_straight_depths) {
		const std::string& feature_name = kv.first;
		const auto& view_samples = kv.second;
		
		// take straight depths of this feature, for all views
		std::vector<real> straight_depths;
		for(const auto& kv2 : view_samples) {
			real d = kv2.second;
			straight_depths.push_back(d);
		}
		
		// calculate median
		auto med_it = straight_depths.begin() + straight_depths.size()/2;
		std::nth_element(straight_depths.begin(), med_it, straight_depths.end());
		real median = *med_it;
		
		// filter out outliers
		const real outlier_threshold = 10.0;
		std::vector<real> filtered_straight_depths;
		for(real d : straight_depths)
			if(std::abs(d - median) < outlier_threshold) filtered_straight_depths.push_back(d);
		std::size_t count = filtered_straight_depths.size();

		// average and stddev of remaining samples
		real avg = 0.0;
		for(real d : filtered_straight_depths) avg += d;
		avg /= count;
		real stddev = 0.0;
		for(real d : filtered_straight_depths) stddev += sq(avg - d);
		avg_stddev += stddev / count;
		stddev = std::sqrt(stddev / count);
		
		if(verbose) {
			std::cout << feature_name << ":\n";
			std::cout << "    median: " << median << "\n";
			std::cout << "    avg: " << avg << "\n";
			std::cout << "    stddev: " << stddev << "\n";
			std::cout << "    count: " << count << "\n";
			std::cout << "    filtered out: " << view_samples.size()-count << "\n\n";
		}
		
		avg_straight_depths[feature_name] = avg;
	}
	avg_stddev = std::sqrt(avg_stddev / avg_straight_depths.size());
	std::cout << "average stddev: " << avg_stddev << std::endl;
	
	
	std::cout << "saving feature straights depths" << std::endl;
	{
		json j_feature_depths = json::object();
		for(const auto& kv : avg_straight_depths)
			j_feature_depths[kv.first] = kv.second;
		export_json_file(j_feature_depths, out_feature_depths);
	}
}
