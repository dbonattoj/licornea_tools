#include "../lib/common.h"
#include "../lib/args.h"
#include "../lib/json.h"
#include "../lib/opencv.h"
#include "../lib/dataset.h"
#include "../lib/misc.h"
#include "../lib/view_homography.h"
#include "lib/image_correspondence.h"
#include <map>
#include <iostream>
#include <vector>

using namespace tlz;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json src_cors.json dst_cors.json out_homographies.json");
	dataset datas = dataset_arg();
	image_correspondences src_cors = image_correspondences_arg();
	image_correspondences dst_cors = image_correspondences_arg();
	std::string out_homographies_filename = out_filename_arg();
	
	// collect names of features present in both cors sets
	std::vector<std::string> feature_names;
	for(const auto& kv : src_cors.features) {
		const std::string& feature_name = kv.first;
		if(dst_cors.features.find(feature_name) != dst_cors.features.end())
			feature_names.push_back(feature_name);
	}
	
	view_homographies homographies;
	
	// for each view
	for(int x = datas.x_min(); x <= datas.x_max(); x += datas.x_step())
	for(int y = datas.y_min(); y <= datas.y_max(); y += datas.y_step()) {
		view_index idx(x, y);
		
		std::vector<vec2> src_points, dst_points;
		
		// collect feature points present in both views
		for(const std::string& feature_name : feature_names) {
			const image_correspondence_feature& src_feature = src_cors.features.at(feature_name);
			const image_correspondence_feature& dst_feature = dst_cors.features.at(feature_name);
			
			auto src_point_it = src_feature.points.find(idx);
			if(src_point_it == src_feature.points.end()) continue;
			
			auto dst_point_it = dst_feature.points.find(idx);
			if(dst_point_it == dst_feature.points.end()) continue;

			src_points.push_back(src_point_it->second);
			dst_points.push_back(dst_point_it->second);
		}

		// compute homography
		mat33 H = cv::findHomography(src_points, dst_points);
		
		// compute back-projection error
		std::vector<vec2> warped_src_points;
		cv::perspectiveTransform(src_points, warped_src_points, H);
		std::size_t n = src_points.size();
		real err = 0.0;
		for(std::ptrdiff_t i = 0; i < n; ++i) {
			vec2 src_pt = src_points[i];			
			vec2 warp_src_pt = warped_src_points[i];
			err += sq(src_pt[0] - warp_src_pt[0]) + sq(src_pt[1] - warp_src_pt[1]);
		}
		err /= n;
		
		view_homography homography;
		homography.mat = H;
		homography.err = err;
		homographies[idx] = homography;
	}
	
	// save homographies
	export_json_file(encode_view_homographies(homographies), out_homographies_filename);
}
