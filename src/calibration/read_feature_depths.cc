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
#include "../lib/image_io.h"
#include "../lib/json.h"
#include "../lib/opencv.h"

using namespace tlz;


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json in_cors.json out_cors.json");
	dataset datas = dataset_arg();
	image_correspondences cors = image_correspondences_arg();
	std::string out_cors_filename = out_filename_arg();
			
	std::cout << "for each view, reading feature depths" << std::endl;
	
	std::set<view_index> views = all_views(cors);
	for(view_index view_idx : views) {
		std::string depth_filename = datas.view(view_idx).depth_filename();
		cv::Mat_<ushort> depth = load_depth(depth_filename);
		std::cout << '.' << std::flush;
				
		for(auto& kv : cors.features) {
			const std::string& feature_name = kv.first;
			auto& fpoints = kv.second.points;
			if(fpoints.find(view_idx) == fpoints.end()) continue;
			
			feature_point& fpoint = fpoints.at(view_idx);
			int x = fpoint.position[0], y = fpoint.position[1];
			if(x < 0 || x >= depth.cols || y < 0 || y >= depth.rows) continue;
			ushort depth_value = depth(y, x);
			
			if(depth_value != 0) fpoint.depth = depth_value;
		}

	}
	
	std::cout << "saving correspondences with depths" << std::endl;
	export_json_file(encode_image_correspondences(cors), out_cors_filename);	
}
