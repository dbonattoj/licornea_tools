#ifndef LICORNEA_CG_FEATURE_POINTS_H_
#define LICORNEA_CG_FEATURE_POINTS_H_

#include "../../../lib/json.h"
#include "../../../lib/intrinsics.h"
#include "../../../lib/dataset.h"
#include "../image_correspondence.h"
#include <map>
#include <string>

namespace tlz {

struct feature_point {
	vec2 distorted_point;
	vec2 undistorted_point;
};
struct feature_points {
	std::map<std::string, feature_point> points;
	view_index view_idx;
};

feature_points decode_feature_points(const json&);
json encode_feature_points(const feature_points&);

feature_points feature_points_for_view(const image_correspondences& cors, view_index idx, const intrinsics&);

cv::Mat_<cv::Vec3b> visualize_feature_points(const feature_points&, const cv::Mat_<cv::Vec3b>& back_img);

}

#endif
