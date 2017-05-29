#include "feature_slopes.h"
#include "../../../lib/random_color.h"
#include "../../../lib/assert.h"
#include <vector>

namespace tlz {

feature_slopes::feature_slopes(const feature_points& fpoints) : feature_points(fpoints) {
	Assert(! fpoints.is_distorted, "feature_slopes can only be created for undistorted feature points");
}


feature_slopes decode_feature_slopes(const json& j_fslopes) {
	feature_points fpoints = decode_feature_points(j_fslopes);
	feature_slopes fslopes(fpoints);
	const json& j_fslopes_feat = j_fslopes["features"];
	for(auto it = j_fslopes_feat.begin(); it != j_fslopes_feat.end(); ++it) {
		const std::string& feature_name = it.key();
		const json& j_fslope = it.value();
		feature_slope& fslope = fslopes.slopes[feature_name];
		fslope.horizontal = j_fslope["slope_horizontal"];
		fslope.vertical = j_fslope["slope_vertical"];
	}
	return fslopes;
}


bool has_feature_slopes(const json& j_fslopes) {
	return has(j_fslopes["features"].begin().value(), "slope_horizontal");
}


json encode_feature_slopes(const feature_slopes& fslopes) {
	json j_fslopes = encode_feature_points(fslopes);
	json& j_fslopes_feat = j_fslopes["features"];
	for(const auto& kv : fslopes.slopes) {
		const std::string& feature_name = kv.first;
		const feature_slope& fslope = kv.second;
		json& j_fslope = j_fslopes_feat[feature_name];
		j_fslope["slope_horizontal"] = fslope.horizontal;
		j_fslope["slope_vertical"] = fslope.vertical;		
	}
	return j_fslopes;
}


real model_horizontal_slope(const vec2& undistorted_point, const mat33& K, const mat33& R) {
	real ix = undistorted_point[0], iy = undistorted_point[1];
	real fx = K(0, 0), fy = K(1, 1), cx = K(0, 2), cy = K(1, 2);
	real r11 = R(0, 0), r21 = R(1, 0), r31 = R(2, 0);
	return (fy*r21 + cy*r31 - iy*r31) / (fx*r11 + cx*r31 - ix*r31);
}


real model_vertical_slope(const vec2& undistorted_point, const mat33& K, const mat33& R) {
	real ix = undistorted_point[0], iy = undistorted_point[1];
	real fx = K(0, 0), fy = K(1, 1), cx = K(0, 2), cy = K(1, 2);
	real r12 = R(0, 1), r22 = R(1, 1), r32 = R(2, 1);
	return (fx*r12 + cx*r32 - ix*r32) / (fy*r22 + cy*r32 - iy*r32);
}


cv::Mat_<cv::Vec3b> visualize_feature_slopes(const feature_slopes& fslopes, const cv::Mat_<cv::Vec3b>& back_img, int width, real exaggeration, int thickness) {
	cv::Mat_<cv::Vec3b> out_img;
	back_img.copyTo(out_img);

	int radius = width / 2;
	
	int i = 0;
	for(const auto& kv : fslopes.slopes) {
		const std::string& feature_name = kv.first;
		const vec2& fpoint = fslopes.points.at(feature_name);
		const feature_slope& fslope = kv.second;

		cv::Point center_point = vec2_to_point(fpoint);
		cv::Vec3b col = random_color(i++);
		
		real hslope = fslope.horizontal * exaggeration;
		real vslope = fslope.vertical * exaggeration;
		

		// draw near-horizontal line
		{
			std::vector<cv::Point> end_points(2);
			end_points[0] = cv::Point(center_point.x - radius, center_point.y - radius*hslope);
			end_points[1] = cv::Point(center_point.x + radius, center_point.y + radius*hslope);
			std::vector<std::vector<cv::Point>> polylines { end_points };
			cv::polylines(out_img, polylines, false, cv::Scalar(col), thickness);
		}
	
	
		// draw near-vertical line
		{
			std::vector<cv::Point> end_points(2);
			end_points[0] = cv::Point(center_point.x - radius*vslope, center_point.y - radius);
			end_points[1] = cv::Point(center_point.x + radius*vslope, center_point.y + radius);
			std::vector<std::vector<cv::Point>> polylines { end_points };
			cv::polylines(out_img, polylines, false, cv::Scalar(col), thickness);
		}	
	}
	
	return out_img;
}


}
