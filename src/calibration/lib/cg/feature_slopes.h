#ifndef LICORNEA_CG_FEATURE_SLOPES_H_
#define LICORNEA_CG_FEATURE_SLOPES_H_

#include "../../lib/json.h"
#include "feature_points.h"
#include <map>
#include <string>

namespace tlz {

struct feature_slope : feature_point {
	real horizontal = NAN;
	real vertical = NAN;
};
struct feature_slopes : feature_points {
	std::map<std::string, feature_slope> slopes;

	feature_slopes() = default;
	feature_slopes(const feature_points& fpoints) : feature_points(fpoints) { }
};

feature_slopes decode_feature_slopes(const json&);
bool has_feature_slopes(const json&);

json encode_feature_slopes(const feature_slopes&);

real model_horizontal_slope(const vec2& undistorted_point, const mat33& K, const mat33& R);
real model_vertical_slope(const vec2& undistorted_point, const mat33& K, const mat33& R);

cv::Mat_<cv::Vec3b> visualize_feature_slopes(const feature_slopes&, const cv::Mat_<cv::Vec3b>& back_img, int width = 200, real exaggeration = 1.0, int thickness = 2);

}

#endif
