#ifndef LICORNEA_INTRINSICS_H_
#define LICORNEA_INTRINSICS_H_

#include "opencv.h"
#include "json.h"

namespace tlz {

struct intrinsics {
	mat33 K;
	mat33 K_inv;
	
	struct {
		real k1 = 0.0;
		real k2 = 0.0;
		real k3 = 0.0;
		real p1 = 0.0;
		real p2 = 0.0;
	
		std::vector<real> cv_coeffs() const {
			return { k1, k2, p1, p2, k3 };
		}
		
		bool is_none() const {
			return (k1 == 0.0) && (k2 == 0.0) && (k3 == 0.0) && (p1 == 0.0) && (p2 == 0.0);
		}
		explicit operator bool () const { return ! is_none(); }
	} distortion;
	int width = 0;
	int height = 0;
	
	real fx() const { return K(0, 0); }
	real fy() const { return K(1, 1); }
	real cx() const { return K(0, 2); }
	real cy() const { return K(1, 2); }

};	

intrinsics decode_intrinsics(const json&);
json encode_intrinsics(const intrinsics&);

vec2 undistort_point(const intrinsics&, const vec2& distorted);
std::vector<vec2> undistort_points(const intrinsics&, const std::vector<vec2>& distorted);
std::vector<cv::Vec2f> undistort_points(const intrinsics&, const std::vector<cv::Vec2f>& distorted);

vec2 distort_point(const intrinsics&, const vec2& undistorted);
std::vector<vec2> distort_points(const intrinsics&, const std::vector<vec2>& undistorted);

}

#endif

