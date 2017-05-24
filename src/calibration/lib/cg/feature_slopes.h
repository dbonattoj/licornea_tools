#ifndef LICORNEA_CG_FEATURE_SLOPES_H_
#define LICORNEA_CG_FEATURE_SLOPES_H_

#include "../../lib/json.h"
#include "../../lib/opencv.h"
#include "feature_points.h"
#include <map>
#include <string>

namespace tlz {

struct feature_slope : feature_point {
	feature_slope() = default;
	feature_slope(const feature_slope&) = default;
	feature_slope(const feature_point& fpoint) : feature_point(fpoint) { }
	
	real horizontal;
	real vertical;
};
using feature_slopes = std::map<std::string, feature_slope>;

feature_points to_feature_points(const feature_slopes&);

feature_slopes decode_feature_slopes(const json&);
json encode_feature_slopes(const feature_slopes&);

real model_horizontal_slope(const vec2& undistorted_point, const mat33& K, const mat33& R);
real model_vertical_slope(const vec2& undistorted_point, const mat33& K, const mat33& R);

cv::Mat_<cv::Vec3b> visualize_feature_slopes(const feature_slopes&, const cv::Mat_<cv::Vec3b>& back_img, int width = 200, real exaggeration = 1.0);

}

#endif
