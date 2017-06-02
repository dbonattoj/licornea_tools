#include "feature_points.h"
#include "../../../lib/random_color.h"
#include "../../../lib/json.h"
#include "../../../lib/string.h"

namespace tlz {

feature_points decode_feature_points(const json& j_fpoints) {
	feature_points fpoints;
	fpoints.view_idx = decode_view_index(j_fpoints["view_idx"]);
	fpoints.is_distorted = get_or(j_fpoints, "is_distorted", false);
	const json& j_fpoints_feat = j_fpoints["features"];
	for(auto it = j_fpoints_feat.begin(); it != j_fpoints_feat.end(); ++it) {
		const std::string& feature_name = it.key();
		const json& j_fpoint = it.value();
		fpoints.points[feature_name] = vec2(j_fpoint["x"], j_fpoint["y"]);
	}
	return fpoints;
}


json encode_feature_points(const feature_points& fpoints) {	
	json f_fpoints_feat = json::object();
	for(const auto& kv : fpoints.points) {
		const std::string& feature_name = kv.first;
		const vec2& fpoint = kv.second;
		json j_fpoint = json::object();
		j_fpoint["x"] = fpoint[0];
		j_fpoint["y"] = fpoint[1];
		f_fpoints_feat[feature_name] = j_fpoint;
	}
	
	json j_fpoints = json::object();
	j_fpoints["view_idx"] = encode_view_index(fpoints.view_idx);
	j_fpoints["features"] = f_fpoints_feat;
	j_fpoints["is_distorted"] = fpoints.is_distorted;

	return j_fpoints;
}


feature_points feature_points_for_view(const image_correspondences& cors, view_index idx) {
	feature_points fpoints;
	fpoints.view_idx = idx;
	fpoints.is_distorted = true;
	
	for(const auto& kv : cors.features) {
		const std::string& feature_name = kv.first;
		const image_correspondence_feature& feature = kv.second;
		auto point_it = feature.points.find(idx);
		if(point_it != feature.points.end()) fpoints.points[feature_name] = point_it->second;
	}
	
	return fpoints;
}


feature_points undistort(const feature_points& dist_fpoints, const intrinsics& intr) {
	if(! dist_fpoints.is_distorted) return dist_fpoints;
	if(! intr.distortion) return dist_fpoints;
	
	feature_points undist_fpoints;
	undist_fpoints.is_distorted = false;
	undist_fpoints.view_idx = dist_fpoints.view_idx;
	
	std::vector<vec2> distorted_points;
	std::vector<vec2*> undistorted_point_ptrs;
	for(const auto& kv : dist_fpoints.points) {
		const std::string& feature_name = kv.first;
		const vec2 dist_pt = kv.second;
		vec2& undist_pt = undist_fpoints.points[feature_name];
		distorted_points.push_back(dist_pt);
		undistorted_point_ptrs.push_back(&undist_pt);
	}
	
	std::vector<vec2> undistorted_points = undistort_points(intr, distorted_points);
	for(std::ptrdiff_t idx = 0; idx < distorted_points.size(); ++idx)
		*(undistorted_point_ptrs[idx]) = undistorted_points[idx];

	return undist_fpoints;
}


feature_points undistorted_feature_points_for_view(const image_correspondences& cors, view_index idx, const intrinsics& intr) {
	feature_points fpoints = feature_points_for_view(cors, idx);
	if(intr.distortion) {
		return undistort(fpoints, intr);
	} else {
		fpoints.is_distorted = false;
		return fpoints;
	}
}



cv::Mat_<cv::Vec3b> visualize_feature_points(const feature_points& fpoints, const cv::Mat_<cv::Vec3b>& back_img, const border& bord) {
	cv::Mat_<cv::Vec3b> img;
	back_img.copyTo(img);
	
	for(const auto& kv : fpoints.points) {
		const std::string& feature_name = kv.first;
		const vec2& fpoint = kv.second;
		cv::Vec3b col = random_color(string_hash(feature_name));
		cv::Point center_point = vec2_to_point(fpoint);
		center_point.x += bord.left;
		center_point.y += bord.top;
		cv::circle(img, center_point, 10, cv::Scalar(col), 2);
		cv::Point label_point(center_point.x + 10, center_point.y - 10);
		cv::putText(img, feature_name, label_point, cv::FONT_HERSHEY_COMPLEX_SMALL, 1.0, cv::Scalar(col));
	}
	return img;
}


}
