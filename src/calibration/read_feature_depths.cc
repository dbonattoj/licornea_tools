#include <iostream>
#include <utility>
#include <vector>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <set>
#include <map>
#include <atomic>
#include "lib/image_correspondence.h"
#include "../lib/args.h"
#include "../lib/misc.h"
#include "../lib/dataset.h"
#include "../lib/image_io.h"
#include "../lib/json.h"
#include "../lib/opencv.h"
#include "../lib/filesystem.h"

using namespace tlz;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json in_cors.json out_cors.json [xy_outreach=0]");
	dataset datas = dataset_arg();
	image_correspondences cors = image_correspondences_arg();
	std::string out_cors_filename = out_filename_arg();
	int xy_outreach = int_opt_arg(0);
			
	std::cout << "for each view, reading feature depths" << std::endl;
	
	auto views = get_all_views(cors);
	
	std::atomic<int> counter(0);
	#pragma omp parallel for
	for(std::ptrdiff_t i = 0; i < views.size(); ++i) {
		const view_index& view_idx = views[i];
		
		std::string depth_filename = datas.view(view_idx).depth_filename();
		if(! file_exists(depth_filename)) continue;
		cv::Mat_<ushort> depth = load_depth(depth_filename);
		
				
		for(auto& kv : cors.features) {
			const std::string& feature_name = kv.first;
			auto& fpoints = kv.second.points;
			if(fpoints.find(view_idx) == fpoints.end()) continue;
			
			feature_point& fpoint = fpoints.at(view_idx);
			int x = fpoint.position[0], y = fpoint.position[1];
			
			if(xy_outreach == 0) {
				if(x < 0 || x >= depth.cols || y < 0 || y >= depth.rows) continue;
				ushort depth_value = depth(y, x);
				if(depth_value != 0) fpoint.depth = depth_value;
				
			} else {
				ushort max_depth_value = 0.0;
				for(int y_ = y - xy_outreach; y_ <= y + xy_outreach; y_++)
				for(int x_ = x - xy_outreach; x_ <= x + xy_outreach; x_++) {
					if(x_ < 0 || x_ >= depth.cols || y_ < 0 || y_ >= depth.rows) continue;
					ushort depth_value = depth(y, x);
					if(depth_value > max_depth_value) max_depth_value = depth_value;
				}
				if(max_depth_value != 0) fpoint.depth = max_depth_value;
				
			}
		}

		std::cout << '.' << std::flush;
		
		++counter;
		if(counter % 1000 == 0) {
			#pragma omp critical
			std::cout << '\n' << counter << " of " << views.size() << std::endl;
		}
	}
	
	std::cout << "saving correspondences with depths" << std::endl;
	export_image_corresponcences(cors, out_cors_filename);
}

