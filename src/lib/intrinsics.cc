#include "intrinsics.h"
#include <iostream>

namespace tlz {

intrinsics decode_intrinsics(const json& j_intr) {
	intrinsics intr;
	intr.K = decode_mat(j_intr["K"]);
	intr.K_inv = intr.K.inv();
	if(j_intr.count("distortion") == 1) {
		intr.distortion.k1 = j_intr["distortion"]["k1"];
		intr.distortion.k2 = j_intr["distortion"]["k2"];
		intr.distortion.p1 = j_intr["distortion"]["p1"];
		intr.distortion.p2 = j_intr["distortion"]["p2"];
		if(j_intr["distortion"].count("k3") == 1) intr.distortion.k3 = j_intr["distortion"]["k3"];
	}
	intr.width = j_intr["width"];
	intr.height = j_intr["height"];
	return intr;
}


json encode_intrinsics(const intrinsics& intr) {
	json j_intr;
	j_intr["K"] = encode_mat(intr.K);
	j_intr["distortion"] = json::object();
	j_intr["distortion"]["k1"] = intr.distortion.k1;
	j_intr["distortion"]["k2"] = intr.distortion.k2;
	j_intr["distortion"]["k3"] = intr.distortion.k3;
	j_intr["distortion"]["p1"] = intr.distortion.p1;
	j_intr["distortion"]["p2"] = intr.distortion.p2;
	j_intr["width"] = intr.width;
	j_intr["height"] = intr.height;
	return j_intr;
}


vec2 undistort_point(const intrinsics& intr, const vec2& distorted) {
	std::vector<vec2> distorted_arr { distorted };
	std::vector<vec2> undistorted_arr = undistort_points(intr, distorted_arr);
	return undistorted_arr.front();
}


std::vector<vec2> undistort_points(const intrinsics& intr, const std::vector<vec2>& distorted) {
	std::vector<vec2> undistorted;
	undistorted.reserve(distorted.size());

	cv::undistortPoints(
		distorted,
		undistorted,
		intr.K,
		intr.distortion.cv_coeffs(),
		cv::noArray(),
		intr.K
	);
	
	return undistorted;
}
std::vector<cv::Vec2f> undistort_points(const intrinsics& intr, const std::vector<cv::Vec2f>& distorted) {
	std::vector<cv::Vec2f> undistorted;
	undistorted.reserve(distorted.size());

	cv::undistortPoints(
		distorted,
		undistorted,
		intr.K,
		intr.distortion.cv_coeffs(),
		cv::noArray(),
		intr.K
	);
	
	return undistorted;

}


vec2 distort_point(const intrinsics& intr, const vec2& undistorted) {
	const auto& d = intr.distortion;

	real ix = undistorted[0], iy = undistorted[1];
	
	real x = (ix - intr.cx()) / intr.fx();
	real y = (iy - intr.cy()) / intr.fy();
	
	real rr = x*x + y*y;
	real kr = 1.0 + d.k1*rr + d.k2*rr*rr + d.k3*rr*rr*rr;
	real dx = x * kr;
	real dy = y * kr;
	
	dx += 2.0 * d.p1 * x * y + d.p2*(rr + 2.0 * x * x);
	dy += d.p1 * (rr + 2.0 * y * y) + 2.0 * d.p2 * x * y;

	real idx = intr.fx() * dx + intr.cx();
	real idy = intr.fy() * dy + intr.cy();
	return vec2(idx, idy);
}


std::vector<vec2> distort_points(const intrinsics& intr, const std::vector<vec2>& undistorted) {
	std::vector<vec2> distorted(undistorted.size());

	#pragma omp parallel for
	for(std::ptrdiff_t i = 0; i < undistorted.size(); ++i)
		distorted[i] = distort_point(intr, undistorted[i]);
	
	return distorted;
}



}
