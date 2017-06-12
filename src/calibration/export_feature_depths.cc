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
#include "../lib/json.h"

using namespace tlz;


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json in_cors.json depths.txt [y_index]");
	dataset datas = dataset_arg();
	image_correspondences cors = image_correspondences_arg();
	std::string depths_filename = out_filename_arg();
	int y = int_opt_arg(-1);
	
	if(datas.is_2d() && y == -1) throw std::runtime_error("must specify y index for 2D dataset");
	
	std::cout << "saving feature depths" << std::endl;
	std::ofstream depths_stream(depths_filename);
	
	depths_stream << 'X';
	for(int x : datas.x_indices()) depths_stream << ' ' << x;
	depths_stream << '\n';
	
	for(const auto& kv : cors.features) {
		const std::string& feature_name = kv.first;
		const image_correspondence_feature& feature = kv.second;
		depths_stream << feature_name;
		for(int x : datas.x_indices()) {
			depths_stream << ' ';
			view_index view_idx(x, y);
			
			auto pt_it = feature.points.find(view_idx);
			if(pt_it == feature.points.end()) continue;
			const feature_point& pt = pt_it->second;
						
			real depth = pt.depth;
			if(depth != 0.0) depths_stream << depth; 
		}
		depths_stream << '\n';
	}	
	std::cout << "done" << std::endl;
}

