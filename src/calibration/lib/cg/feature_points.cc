#include "feature_points.h"
#include "../../../lib/random_color.h"

namespace tlz {

feature_points decode_feature_points(const json& j_fpoints) {
	feature_points fpoints;
	fpoints.view_idx = decode_view_index(j_fpoints["view_idx"]);
	const json& f_fpoints_feat = j_fpoints["features"];
	for(auto it = f_fpoints_feat.begin(); it != f_fpoints_feat.end(); ++it) {
		const std::string& feature_name = it.key();
		const json& j_fpoint = it.value();
		feature_point fpoint;
		fpoint.distorted_point[0] = j_fpoint["dist_ix"];
		fpoint.distorted_point[1] = j_fpoint["dist_iy"];
		fpoint.undistorted_point[0] = j_fpoint["undist_ix"];
		fpoint.undistorted_point[1] = j_fpoint["undist_iy"];
		fpoints.points[feature_name] = fpoint;
	}
	return fpoints;
}


json encode_feature_points(const feature_points& fpoints) {	
	json f_fpoints_feat = json::object();
	for(const auto& kv : fpoints.points) {
		const std::string& feature_name = kv.first;
		const feature_point& fpoint = kv.second;
		json j_fpoint = json::object();
		j_fpoint["dist_ix"] = fpoint.distorted_point[0];
		j_fpoint["dist_iy"] = fpoint.distorted_point[1];
		j_fpoint["undist_ix"] = fpoint.undistorted_point[0];
		j_fpoint["undist_iy"] = fpoint.undistorted_point[1];
		f_fpoints_feat[feature_name] = j_fpoint;
	}
	
	json j_fpoints = json::object();
	j_fpoints["view_idx"] = encode_view_index(fpoints.view_idx);
	j_fpoints["features"] = f_fpoints_feat;

	return j_fpoints;
}


feature_points feature_points_for_view(const image_correspondences& cors, view_index idx, const intrinsics& intr) {
	feature_points fpoints;
	fpoints.view_idx = idx;
	
	std::vector<vec2> distorted_points;
	std::vector<vec2*> undistorted_point_ptrs;
	for(const auto& kv : cors.features) {
		const std::string& feature_name = kv.first;
		const image_correspondence_feature& feature = kv.second;
		vec2 distorted_point = feature.points.at(idx);
		
		feature_point& fpoint = fpoints.points[feature_name];
		fpoint.distorted_point = distorted_point;
		distorted_points.push_back(distorted_point);
		undistorted_point_ptrs.push_back(&fpoint.undistorted_point);
	}
	
	std::vector<vec2> undistorted_points = undistort_points(intr, distorted_points);
	for(std::ptrdiff_t idx = 0; idx < distorted_points.size(); ++idx)
		*(undistorted_point_ptrs[idx]) = undistorted_points[idx];
		
	return fpoints;
}


cv::Mat_<cv::Vec3b> visualize_feature_points(const feature_points& fpoints, const cv::Mat_<cv::Vec3b>& back_img) {
	cv::Mat_<cv::Vec3b> img;
	back_img.copyTo(img);
	
	int i = 0;
	for(const auto& kv : fpoints.points) {
		const std::string& feature_name = kv.first;
		const feature_point& fpoint = kv.second;
		cv::Vec3b col = random_color(i++);
		cv::Point center_point = vec2_to_point(fpoint.undistorted_point);
		cv::circle(img, center_point, 10, cv::Scalar(col), 2);
		cv::Point label_point(center_point.x + 10, center_point.y - 10);
		cv::putText(img, feature_name, label_point, cv::FONT_HERSHEY_COMPLEX_SMALL, 1.0, cv::Scalar(col));
	}
	return img;
}


}
