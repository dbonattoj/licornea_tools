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
#include "../lib/dataset.h"
#include "../lib/intrinsics.h"
#include "../lib/image_io.h"
#include "../lib/json.h"
#include "../lib/opencv.h"

using namespace tlz;

constexpr bool verbose = true;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json image_correspondences.json intrinsics.json R.json out_feature_depths.json");
	dataset datas = dataset_arg();
	image_correspondences cors = image_correspondences_arg();
	intrinsics intr = intrinsics_arg();
	mat33 R = decode_mat(json_arg());
	std::string out_feature_depths = out_filename_arg();

	struct sample {
		vec2 coordinates;
		real measured_depth;
		real straight_depth;
	};
	std::map<std::string, std::map<view_index, sample>> feature_samples;

	std::cout << "for each view, reading feature depths" << std::endl;
	for(int x : datas.x_indices()) for(int y : datas.y_indices()) {
		view_index view_idx(x, y);
		
		std::string depth_filename = datas.view(view_idx).depth_filename();
		cv::Mat_<ushort> depth = load_depth(depth_filename);
		std::cout << '.' << std::flush;
		
		for(const auto& kv : cors.features) {
			const std::string& feature_name = kv.first;
			const auto& feature_points = kv.second.points;
			if(feature_points.find(view_idx) == feature_points.end()) continue;
			
			vec2 pos = feature_points.at(view_idx);
			int x = pos[0], y = pos[1];
			if(x < 0 || x >= depth.cols || y < 0 || y >= depth.rows) continue;
			ushort depth_value = depth(y, x);
			
			if(depth_value == 0) continue;
			
			sample samp;
			samp.coordinates = vec2(x, y);
			samp.measured_depth = depth_value;
			feature_samples[feature_name][view_idx] = samp;
		}
	}
	std::cout << std::endl;
	
	
	std::cout << "computing feature straight depths" << std::endl;
	for(auto& kv : feature_samples) for(auto& kv2 : kv.second) {
		sample& samp = kv2.second;
		
		vec2 i_measured_dist = samp.coordinates;
		vec2 i_measured_undist = undistort_point(intr, i_measured_dist);
		
		real d_measured = samp.measured_depth;
		
		vec3 i_measured_h(i_measured_undist[0], i_measured_undist[1], 1.0);
		i_measured_h *= d_measured;
		 
		vec3 v_measured = intr.K_inv * i_measured_h;
		vec3 v_straight = R.t() * v_measured;
		real d_straight = v_straight[2];
				
		samp.straight_depth = d_straight;
	}
	

	std::cout << "aggregating feature straight depths" << std::endl;
	std::map<std::string, real> feature_avg_depths;
	real avg_stddev = 0.0;
	for(auto& kv : feature_samples) {
		const std::string& feature_name = kv.first;
		const auto& view_samples = kv.second;
		
		// take all depths
		std::vector<real> straight_depths;
		for(const auto& kv2 : view_samples) {
			real d = kv2.second.straight_depth;
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
		
		feature_avg_depths[feature_name] = avg;
	}
	avg_stddev = std::sqrt(avg_stddev / feature_avg_depths.size());
	std::cout << "average stddev: " << avg_stddev << std::endl;
	
	
	std::cout << "saving feature straights depths" << std::endl;
	{
		json j_feature_depths = json::object();
		for(const auto& kv : feature_avg_depths)
			j_feature_depths[kv.first] = kv.second;
		export_json_file(j_feature_depths, out_feature_depths);
	}
}
