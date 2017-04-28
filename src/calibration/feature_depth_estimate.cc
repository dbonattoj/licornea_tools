#include <opencv2/opencv.hpp>
#include <iostream>
#include <utility>
#include <vector>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <set>
#include <map>
#include "lib/image_correspondence.h"
#include "../lib/utility/misc.h"
#include "../lib/dataset.h"
#include "../lib/image_io.h"
#include "../lib/json.h"

using namespace tlz;


[[noreturn]] void usage_fail() {
	std::cout << "usage: feature_depth_estimate dataset_parameters.json in_image_correspondences.json out_image_correspondences.json [depths.txt]\n";
	std::cout << std::endl;
	std::exit(1);
}

int main(int argc, const char* argv[]) {
	if(argc <= 3) usage_fail();
	std::string dataset_parameter_filename = argv[1];
	std::string in_cors_filename = argv[2];
	std::string out_cors_filename = argv[3];
	std::string depths_filename;
	if(argc > 4) depths_filename = argv[4];
	
	float stddev_max_threshold = 10.0;
	
	std::cout << "loading data set" << std::endl;
	dataset datas(dataset_parameter_filename);
	
	std::map<view_index, std::vector<std::string>> view_features;
	std::map<std::string, std::vector<float>> feature_depths;

	std::cout << "loading correspondences" << std::endl;
	image_correspondences cors = import_image_correspondences_file(in_cors_filename);
	int points_count = cors.features.begin()->second.points.size();

	std::cout << "preloading feature positions per view" << std::endl;
	for(const auto& kv : cors.features) {
		const std::string& feature_name = kv.first;
		const image_correspondence_feature& feature = kv.second;
		for(const auto& kv2 : feature.points) {
			view_index idx = kv2.first;
			view_features[idx].push_back(feature_name);
		}
	}

	std::cout << "for each view, reading feature depths" << std::endl;
	for(const auto& kv : view_features) {
		view_index view_idx = kv.first;	
		const auto& features = kv.second;
		
		std::string depth_filename = datas.view(view_idx).depth_filename();
		cv::Mat_<ushort> depth = load_depth(depth_filename);
		std::cout << '.' << std::flush;
		
		for(const std::string& feature_name : features) {
			Eigen_vec2 j_pos = cors.features[feature_name].points[view_idx];
			cv::Point pos(j_pos[0], j_pos[1]);
			
			ushort depth_value = depth(pos);
			if(depth_value != 0) feature_depths[feature_name].push_back(depth_value);
		}
	}
	std::cout << std::endl;
	
	image_correspondences out_cors = cors;
	out_cors.features.clear();
	
	std::cout << "aggregating depths per feature" << std::endl;
	int accepted_count = 0;
	for(auto& kv : feature_depths) {		
		const std::string& feature_name = kv.first;
		auto& depths = kv.second;

		std::cout << "feature " << feature_name << ":" << std::endl;

		if(depths.size() != points_count) { std::cout << "missing samples, rejected\n" << std::endl; continue; }
				
		float mean = 0.0;
		for(float d : depths) mean += d;
		mean /= depths.size();
	
		float stddev = 0.0;
		for(float d : depths) stddev += sq(mean - d);
		stddev = std::sqrt(stddev / depths.size());
	
		std::cout << "mean: " << mean << "\nstandard deviation: " << stddev << std::endl;

		bool accept = (stddev < stddev_max_threshold);
		if(! accept) { std::cout << "rejected\n" << std::endl; continue; }
		else accepted_count++;

		auto mid = depths.begin() + depths.size()/2;
		std::nth_element(depths.begin(), mid, depths.end());
		float median = *mid;
		
		std::cout << "accepted, taking median depth " << median << std::endl << std::endl; 
	
		out_cors.features[feature_name] = cors.features[feature_name];
		out_cors.features[feature_name].depth = median;
	}
	
	std::cout << "accepted " << accepted_count << " of " << feature_depths.size() << " features" << std::endl;
	std::cout << "saving output correspondences" << std::endl;
	export_image_correspondences_file(out_cors_filename, out_cors);
	
	if(! depths_filename.empty()) {
		std::cout << "saving all depths" << std::endl;
		std::ofstream depths_stream(depths_filename);
		for(const auto& kv : feature_depths) {
			const std::string& feature_name = kv.first;
			bool accepted = (out_cors.features.find(feature_name) != out_cors.features.end());
			if(! accepted) continue;
			
			const auto& depths = kv.second;
			depths_stream << feature_name;
			for(float d : kv.second) depths_stream << ' ' << d;
			depths_stream << '\n';
		}
		for(const auto& kv : feature_depths) {
			const std::string& feature_name = kv.first;
			bool accepted = (out_cors.features.find(feature_name) != out_cors.features.end());
			if(accepted) continue;
			
			const auto& depths = kv.second;
			depths_stream << feature_name << "_rej";
			for(float d : kv.second) depths_stream << ' ' << d;
			depths_stream << '\n';
		}
	}
			
	std::cout << "done" << std::endl;
}

