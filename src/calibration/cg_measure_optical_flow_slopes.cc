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

real measure_horizontal_slope(const image_correspondence_feature& feature, int y_outreach) {
	std::vector<cv::Vec2f> points;
	for(const auto& kv : feature.points) {
		const view_index& idx = kv.first;
		const feature_point& fpoint = kv.second;
		if(std::abs(idx.y - feature.reference_view.y) <= y_outreach)
			points.push_back(fpoint.position);
	}

	cv::Vec4f line_parameters;
	cv::fitLine(points, line_parameters, CV_DIST_L2, 0.0, 0.01, 0.01);

	return line_parameters[1] / line_parameters[0];
}

real measure_vertical_slope(const image_correspondence_feature& feature) {
	std::vector<cv::Vec2f> points;
	for(const auto& kv : feature.points) {
		const view_index& idx = kv.first;
		const feature_point& fpoint = kv.second;
		if(idx.x == feature.reference_view.x)
			points.push_back(fpoint.position);
	}
	
	cv::Vec4f line_parameters;
	cv::fitLine(points, line_parameters, CV_DIST_L2, 0.0, 0.01, 0.01);

	return line_parameters[0] / line_parameters[1];
}


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json image_correspondences.json intrinsics.json out_slopes.json");
	dataset datas = dataset_arg();
	image_correspondences dist_cors = image_correspondences_arg();
	intrinsics intr = intrinsics_arg();
	std::string out_slopes_filename = out_filename_arg();
	int y_outreach = 3;
	
	std::cout << "undistorting image correspondences (if applicable)" << std::endl;
	image_correspondences cors = undistort(dist_cors, intr);
	
	std::cout << "measuring slopes" << std::endl;
	std::map<std::string, real> feature_horizontal_slopes, feature_vertical_slopes;
	for(const auto& kv : cors.features) {
		const std::string& feature_name = kv.first;
		const image_correspondence_feature& feature = kv.second;
		
		feature_horizontal_slopes[feature_name] = measure_horizontal_slope(feature, y_outreach);
		feature_vertical_slopes[feature_name] = measure_vertical_slope(feature);
	
		std::cout << '.' << std::flush;
	}
	std::cout << std::endl;


	std::cout << "saving slopes" << std::endl;
	auto ref_views = reference_views(cors);
	feature_slopes fslopes;
	for(const view_index& ref_idx : ref_views) {
		image_correspondences ref_cors = image_correspondences_with_reference(cors, ref_idx);
		feature_points ref_fpoints = undistorted_feature_points_for_view(ref_cors, ref_idx, intr);
		feature_slopes ref_fslopes(ref_fpoints);
		for(auto& kv : ref_cors.features) {
			const std::string& feature_name = kv.first;
			feature_slope& fslope = ref_fslopes.slopes[feature_name];
			fslope.horizontal = feature_horizontal_slopes.at(feature_name);
			fslope.vertical = feature_vertical_slopes.at(feature_name);
			fslopes = merge_multiview_feature_slopes(fslopes, ref_fslopes);
		}
	}
	export_json_file(encode_feature_slopes(fslopes), out_slopes_filename);
}
