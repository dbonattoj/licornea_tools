#include <string>
#include <map>
#include <cmath>
#include "lib/image_correspondence.h"
#include "../lib/utility/misc.h"
#include "../lib/json.h"
#include "../lib/dataset.h"
#include "../lib/opencv.h"
#include "../lib/intrinsics.h"

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: cg_optical_flow_slopes dataset_parameters.json image_correspondences.json intrinsics.json out_slopes.json\n";
	std::cout << std::endl;
	std::exit(1);
}
int main(int argc, const char* argv[]) {
	if(argc <= 4) usage_fail();
	std::string dataset_parameters_filename = argv[1];
	std::string cors_filename = argv[2];
	std::string intrinsics_filename = argv[3];
	std::string out_slopes_filename = argv[4];
	
	std::cout << "loading data set" << std::endl;
	dataset datas(dataset_parameters_filename);
	
	std::cout << "loading correspondences" << std::endl;
	image_correspondences cors = import_image_correspondences_file(cors_filename);
	view_index reference_idx = cors.reference;

	std::cout << "loading intrinsics, rotation" << std::endl;
	intrinsics intr = decode_intrinsics(import_json_file(intrinsics_filename));
	
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
	json j_feature_slopes = json::object();
	for(const auto& kv : cors.features) {
		const std::string& feature_name = kv.first;
		const image_correspondence_feature& feature = kv.second;
		json j_slope = json::object();
		j_slope["ix"] = feature.points.at(reference_idx)[0];
		j_slope["iy"] = feature.points.at(reference_idx)[1];
		j_slope["horizontal"] = feature_horizontal_slopes.at(feature_name);
		j_slope["vertical"] = feature_vertical_slopes.at(feature_name);
		j_feature_slopes[feature_name] = j_slope;
	}
	json j_slopes = json::object();
	j_slopes["slopes"] = j_feature_slopes;
	j_slopes["view"] = encode_view_index(reference_idx);
	export_json_file(j_slopes, out_slopes_filename);
}
