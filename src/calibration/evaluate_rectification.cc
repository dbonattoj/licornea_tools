#include "../lib/common.h"
#include "../lib/args.h"
#include "../lib/dataset.h"
#include "lib/feature_points.h"
#include "lib/image_correspondence.h"
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <cmath>

using namespace tlz;

const bool verbose = false;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json cors.json");
	dataset datas = dataset_arg();
	image_correspondences cors = image_correspondences_arg();

	std::map<std::string, real> avg_horizontal_disparity, max_horizontal_disparity;
	std::map<std::string, real> avg_vertical_disparity, max_vertical_disparity;

	for(const auto& kv : cors.features) {
		const std::string& feature_name = kv.first;
		const image_correspondence_feature& feature = kv.second;

		std::vector<real> horizontal_y_min(datas.y_max()+1, +INFINITY), horizontal_y_max(datas.y_max()+1, -INFINITY);
		std::vector<real> vertical_x_min(datas.x_max()+1, +INFINITY), vertical_x_max(datas.x_max()+1, -INFINITY);
		
		for(view_index idx : datas.indices()) {
			auto fpoint_it = feature.points.find(idx);
			if(fpoint_it == feature.points.end()) continue;
			vec2 pos = fpoint_it->second.position;
			real x = pos[0], y = pos[1];
			
			if(x < vertical_x_min[idx.x]) vertical_x_min[idx.x] = x;
			if(x > vertical_x_max[idx.x]) vertical_x_max[idx.x] = x;
			
			if(y < horizontal_y_min[idx.y]) horizontal_y_min[idx.y] = y;
			if(y > horizontal_y_max[idx.y]) horizontal_y_max[idx.y] = y;
		}
		
		real horizontal_sum = 0.0, horizontal_count = 0.0, horizontal_max = 0.0;
		real vertical_sum = 0.0, vertical_count = 0.0, vertical_max = 0.0;
		for(view_index idx : datas.indices()) {
			real horizontal_disp = horizontal_y_max[idx.y] - horizontal_y_min[idx.y];
			if(std::isfinite(horizontal_disp)) {
				horizontal_sum += horizontal_disp;
				horizontal_count += 1.0;
				if(horizontal_disp > horizontal_max) horizontal_max = horizontal_disp;
			}
			
			real vertical_disp = vertical_x_max[idx.x] - vertical_x_min[idx.x];
			if(std::isfinite(vertical_disp)) {
				vertical_sum += vertical_disp;
				vertical_count += 1.0;
				if(vertical_disp > vertical_max) vertical_max = vertical_disp;
			}
		}
		avg_horizontal_disparity[feature_name] = horizontal_sum / horizontal_count;
		max_horizontal_disparity[feature_name] = horizontal_max;

		avg_vertical_disparity[feature_name] = vertical_sum / vertical_count;
		max_vertical_disparity[feature_name] = vertical_max;
		
		if(verbose) {
			std::cout << feature_name << ":\n";
			std::cout << "    horizontal avg: " << avg_horizontal_disparity[feature_name] << " px\n";
			std::cout << "    horizontal max: " << max_horizontal_disparity[feature_name] << " px\n";
			std::cout << "    vertical avg: " << avg_vertical_disparity[feature_name] << " px\n";
			std::cout << "    vertical max: " << max_vertical_disparity[feature_name] << " px\n" << std::endl;
		}
	}
	
	
	real avg_avg_horizontal_disparity = 0.0, max_max_horizontal_disparity = 0.0;
	real avg_avg_vertical_disparity = 0.0, max_max_vertical_disparity = 0.0;
	for(const auto& kv : cors.features) {
		const std::string& feature_name = kv.first;
		avg_avg_horizontal_disparity += avg_horizontal_disparity.at(feature_name);
		max_max_horizontal_disparity = std::max(max_max_horizontal_disparity, max_horizontal_disparity.at(feature_name));
		avg_avg_vertical_disparity += avg_vertical_disparity.at(feature_name);
		max_max_vertical_disparity = std::max(max_max_vertical_disparity, max_vertical_disparity.at(feature_name));
	}
	avg_avg_horizontal_disparity /= avg_horizontal_disparity.size();
	avg_avg_vertical_disparity /= avg_vertical_disparity.size();
	
	std::cout << "horizontal avg: " << avg_avg_horizontal_disparity << " px\n";
	std::cout << "horizontal max: " << max_max_horizontal_disparity << " px\n";
	std::cout << "vertical avg: " << avg_avg_vertical_disparity << " px\n";
	std::cout << "vertical max: " << max_max_vertical_disparity << " px" << std::endl;
}

