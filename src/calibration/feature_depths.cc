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
#include "../lib/opencv.h"

using namespace tlz;


[[noreturn]] void usage_fail() {
	std::cout << "usage: feature_depths dataset_parameters.json in_image_correspondences.json depths.txt [y]" << std::endl;
	std::exit(EXIT_FAILURE);
}
int main(int argc, const char* argv[]) {
	if(argc <= 3) usage_fail();
	std::string dataset_parameter_filename = argv[1];
	std::string in_cors_filename = argv[2];
	std::string depths_filename = argv[3];
	int y = -1;
	if(argc > 4) y = std::atoi(argv[4]);
		
	std::cout << "loading data set" << std::endl;
	dataset datas(dataset_parameter_filename);
	
	if(datas.is_2d() && y == -1) throw std::runtime_error("must specify y index for 2D dataset");
		
	std::map<std::string, std::map<view_index, real>> feature_depths;

	std::cout << "loading correspondences" << std::endl;
	image_correspondences cors = import_image_correspondences_file(in_cors_filename);

	std::cout << "for each view, reading feature depths" << std::endl;
	for(int x : datas.x_indices()) {
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
			
			if(depth_value != 0) feature_depths[feature_name][view_idx] = depth_value;
		}
	}
	std::cout << std::endl;
	
	
	std::cout << "saving feature depths" << std::endl;
	std::ofstream depths_stream(depths_filename);
	
	depths_stream << 'X';
	for(int x : datas.x_indices()) depths_stream << ' ' << x;
	depths_stream << '\n';
	
	for(const auto& kv : cors.features) {
		const std::string& feature_name = kv.first;
		depths_stream << feature_name;
		for(int x : datas.x_indices()) {
			depths_stream << ' ';
			view_index view_idx(x, y);
			
			real d = feature_depths[feature_name][view_idx];
			
			if(d != real()) depths_stream << d; 
		}
		depths_stream << '\n';
	}	
	std::cout << "done" << std::endl;
}

