#ifndef LICORNEA_CG_FEATURE_POINTS_H_
#define LICORNEA_CG_FEATURE_POINTS_H_

#include "../../../lib/json.h"
#include "../../../lib/intrinsics.h"
#include "../../../lib/dataset.h"
#include "../../../lib/args.h"
#include "../image_correspondence.h"
#include <map>
#include <string>

namespace tlz {

struct feature_points {
	std::map<std::string, vec2> points;
	view_index view_idx;
	bool is_distorted;
	
	bool has(const std::string& feature_name) const {
		return (points.find(feature_name) != points.end());
	}
};

feature_points decode_feature_points(const json&);
json encode_feature_points(const feature_points&);

feature_points undistort(const feature_points&, const intrinsics&);

feature_points feature_points_for_view(const image_correspondences& cors, view_index idx);
feature_points undistorted_feature_points_for_view(const image_correspondences& cors, view_index idx, const intrinsics&);

cv::Mat_<cv::Vec3b> visualize_feature_points(const feature_points&, const cv::Mat_<cv::Vec3b>& back_img);

inline feature_points feature_points_arg()
	{ return decode_feature_points(json_arg()); }

}

#endif
