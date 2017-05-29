#include <string>
#include <map>
#include <cmath>
#include "lib/image_correspondence.h"
#include "lib/cg/feature_slopes.h"
#include "../lib/misc.h"
#include "../lib/json.h"
#include "../lib/dataset.h"
#include "../lib/opencv.h"
#include "../lib/intrinsics.h"

using namespace tlz;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json image_correspondences.json intrinsics.json out_slopes.json");
	dataset datas = dataset_arg();
	image_correspondences cors = image_correspondences_arg();
	intrinsics intr = intrinsics_arg();
	std::string out_slopes_filename = out_filename_arg();

	view_index reference_idx = cors.reference;
	std::cout << "using reference view idx " << reference_idx << std::endl;
	
	std::cout << "estimating slopes for horizontal flow" << std::endl;
	std::map<std::string, real> feature_horizontal_slopes;
	for(const auto& kv : cors.features) {
		const std::string& feature_name = kv.first;
		const image_correspondence_feature& feature = kv.second;
		
		std::vector<cv::Vec2f> points;
		for(int x : datas.x_indices()) points.push_back(feature.points.at(view_index(x, reference_idx.y)));
		
		points = undistort_points(intr, points);
		
		cv::Vec4f line_parameters;
		cv::fitLine(points, line_parameters, CV_DIST_L2, 0.0, 0.01, 0.01);
				
		feature_horizontal_slopes[feature_name] = line_parameters[1] / line_parameters[0];
	}

	std::cout << "estimating slopes for vertical flow" << std::endl;
	std::map<std::string, real> feature_vertical_slopes;
	for(const auto& kv : cors.features) {
		const std::string& feature_name = kv.first;
		const image_correspondence_feature& feature = kv.second;
		
		std::vector<cv::Vec2f> points;
		for(int y : datas.y_indices()) points.push_back(feature.points.at(view_index(reference_idx.x, y)));
		
		points = undistort_points(intr, points);
		
		cv::Vec4f line_parameters;
		cv::fitLine(points, line_parameters, CV_DIST_L2, 0.0, 0.01, 0.01);
				
		feature_vertical_slopes[feature_name] = line_parameters[0] / line_parameters[1];
	}
	

	
	std::cout << "saving slopes" << std::endl;
	feature_points ref_fpoints = undistorted_feature_points_for_view(cors, reference_idx, intr);
	feature_slopes fslopes = ref_fpoints;
	for(const auto& kv : cors.features) {
		const std::string& feature_name = kv.first;
		
		feature_slope& fslope = fslopes.slopes[feature_name];
		fslope.horizontal = feature_horizontal_slopes.at(feature_name);
		fslope.vertical = feature_vertical_slopes.at(feature_name);
	}
	export_json_file(encode_feature_slopes(fslopes), out_slopes_filename);
}
