#include <opencv2/opencv.hpp>
#include <iostream>
#include <utility>
#include <vector>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <set>
#include <map>
#include "../lib/utility/misc.h"
#include "../lib/dataset.h"
#include "../lib/image_io.h"
#include "../lib/image_correspondence.h"
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
	
	float stddev_max_threshold = 20.0;
	
	std::cout << "loading data set" << std::endl;
	dataset datas(dataset_parameter_filename);
	
	std::map<view_index, std::vector<std::string>> view_features;
	std::map<std::string, std::vector<float>> feature_depths;

	std::cout << "loading correspondences" << std::endl;
	json j_features = import_json_file(in_cors_filename);

	std::cout << "preloading feature positions per view" << std::endl;
	for(json::iterator it = j_features.begin(); it != j_features.end(); ++it) {
		std::string feature_name = it.key();
		image_correspondence_feature feature = decode_image_correspondence_feature(it.value());
		for(const auto& kv : feature.points) {
			view_index idx = kv.first;
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
			const json& j_pos = j_features[feature_name]["points"][view_index_to_key(view_idx)];
			cv::Point pos(j_pos[0], j_pos[1]);
			
			ushort depth_value = depth(pos);
			if(depth_value != 0) feature_depths[feature_name].push_back(depth_value);
		}
	}
	std::cout << std::endl;
	
	std::map<std::string, image_correspondence_feature> output_features;
	
	std::cout << "aggregating depths per feature" << std::endl;
	for(auto& kv : feature_depths) {
		std::cout << std::endl;
		
		const std::string& feature_name = kv.first;
		auto& depths = kv.second;
				
		float mean = 0.0;
		for(float d : depths) mean += d;
		mean /= depths.size();
	
		float stddev = 0.0;
		for(float d : depths) stddev += sq(mean - d);
		stddev = std::sqrt(stddev / depths.size());
	
		std::cout << "feature " << feature_name << ":" << std::endl;
		std::cout << "mean: " << mean << "\nstandard deviation: " << stddev << std::endl;

		bool accept = (stddev < stddev_max_threshold);
		if(! accept) { std::cout << "rejected" << std::endl; continue; }

		auto mid = depths.begin() + depths.size()/2;
		std::nth_element(depths.begin(), mid, depths.end());
		float median = *mid;
		
		std::cout << "accepted, taking median depth " << median << std::endl; 
	
		image_correspondence_feature feature = decode_image_correspondence_feature(j_features[feature_name]);
		feature.depth = median;
		output_features[feature_name] = feature;
	}
	
	std::cout << "saving output correspondences" << std::endl;
	json j_out_features = json::object();
	for(const auto& kv : output_features) {
		const std::string& feature_name = kv.first;
		const image_correspondence_feature& feature = kv.second;
		j_out_features[feature_name] = encode_image_correspondence_feature(feature);
	}
	export_json_file(j_out_features, out_cors_filename);
	
	if(! depths_filename.empty()) {
		std::cout << "saving all depths" << std::endl;
		std::ofstream depths(depths_filename);
		for(const auto& kv : feature_depths) {
			depths << kv.first;
			for(float d : kv.second) depths << ' ' << d;
			depths << '\n';
		}
	}
			
	std::cout << "done" << std::endl;
}

