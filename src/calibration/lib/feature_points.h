#ifndef LICORNEA_FEATURE_POINTS_H_
#define LICORNEA_FEATURE_POINTS_H_

#include "../../lib/json.h"
#include "../../lib/intrinsics.h"
#include "../../lib/dataset.h"
#include "../../lib/args.h"
#include "../../lib/border.h"
#include "feature_point.h"
#include <map>
#include <string>

namespace tlz {
	
struct image_correspondences;

/// Points of different features, on one view.
struct feature_points {
	std::map<std::string, feature_point> points;
	view_index view_idx;
	bool is_distorted = false;
	
	bool has_feature(const std::string& feature_name) const
		{ return (points.find(feature_name) != points.end()); }
	
	std::size_t count() const { return points.size(); }
	
	void normalize_weights();
};

feature_points decode_feature_points(const json&);
json encode_feature_points(const feature_points&);

feature_points undistort(const feature_points&, const intrinsics&);

feature_points feature_points_for_view(const image_correspondences& cors, view_index idx, bool is_distorted = true);
feature_points undistorted_feature_points_for_view(const image_correspondences& cors, view_index idx, const intrinsics&);

std::vector<vec2> positions(const feature_points&);

cv::Mat_<cv::Vec3b> visualize_feature_points(const feature_points&, const cv::Mat_<cv::Vec3b>& back_img, const border& = border());

feature_points feature_points_arg();

}

#endif
