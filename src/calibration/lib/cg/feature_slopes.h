#ifndef LICORNEA_CG_FEATURE_SLOPES_H_
#define LICORNEA_CG_FEATURE_SLOPES_H_

#include "../../../lib/json.h"
#include "../../../lib/args.h"
#include "../../../lib/border.h"
#include "../feature_points.h"
#include <map>
#include <string>

namespace tlz {

struct feature_slope {
	real horizontal = NAN;
	real vertical = NAN;
	
	real horizontal_depth = NAN;
	real vertical_depth = NAN; // TODO
};
struct feature_slopes : feature_points {
	// feature points are undistorted
	std::map<std::string, feature_slope> slopes;

	feature_slopes() = default;
	explicit feature_slopes(const feature_points&);	
};

feature_slopes decode_feature_slopes(const json&);
bool has_feature_slopes(const json&);

json encode_feature_slopes(const feature_slopes&);

real model_horizontal_slope(const vec2& undistorted_point, const mat33& K, const mat33& R);
real model_vertical_slope(const vec2& undistorted_point, const mat33& K, const mat33& R);

cv::Mat_<cv::Vec3b> visualize_feature_slopes(const feature_slopes&, const cv::Mat_<cv::Vec3b>& back_img, int width = 200, real exaggeration = 1.0, int thickness = 2, const border& = border());

inline feature_slopes feature_slopes_arg()
	{ return decode_feature_slopes(json_arg()); }


}

#endif
