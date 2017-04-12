#include "kinect_reprojection.h"
#include "../common.h"
#include "../../../lib/utility/assert.h"

#include <iostream>

namespace tlz {

namespace {
	const float depth_q = 0.01;
	const float color_q = 0.002199;
}

kinect_reprojection::kinect_reprojection(const kinect_intrinsic_parameters& intr) :
	intrinsics_(intr) { }


cv::Vec2f kinect_reprojection::distort_depth(cv::Vec2f undistorted) const {
	const auto& depth_par = intrinsics_.ir;
	float dx = (undistorted[0] - depth_par.cx) / depth_par.fx;
	float dy = (undistorted[1] - depth_par.cy) / depth_par.fy;
	float dx2 = dx * dx;
	float dy2 = dy * dy;
	float r2 = dx2 + dy2;
	float dxdy2 = 2 * dx * dy;
	float kr = 1 + ((depth_par.k3 * r2 + depth_par.k2) * r2 + depth_par.k1) * r2;
	float out_x = depth_par.fx * (dx * kr + depth_par.p2 * (r2 + 2 * dx2) + depth_par.p1 * dxdy2) + depth_par.cx;
	float out_y = depth_par.fy * (dy * kr + depth_par.p1 * (r2 + 2 * dy2) + depth_par.p2 * dxdy2) + depth_par.cy;
	return cv::Vec2f(out_x, out_y);
}


cv::Vec2f kinect_reprojection::undistort_depth(cv::Vec2f distorted) const {
	const auto& depth_par = intrinsics_.ir;
	float dx = distorted[0], dy = distorted[1];
    float ps = (dx * dx) + (dy * dy);
    float qs = ((ps * depth_par.k3 + depth_par.k2) * ps + depth_par.k1) * ps + 1.0;
    for (int i = 0; i < 9; i++) {
        float qd = ps / (qs * qs);
        qs = ((qd * depth_par.k3 + depth_par.k2) * qd + depth_par.k1) * qd + 1.0;
    }
    float mx = dx / qs;
    float my = dy / qs;
	return cv::Vec2f(mx, my);
}


cv::Vec2f kinect_reprojection::reproject_depth_to_color(cv::Vec2f undistorted, float z) const {
	const auto& depth_par = intrinsics_.ir;
	const auto& color_par = intrinsics_.color;
	float mx = undistorted[0], my = undistorted[1];
	
	mx = (mx - depth_par.cx) * depth_q;
	my = (my - depth_par.cy) * depth_q;
	
	float wx =
		(mx * mx * mx * color_par.mx_x3y0) + (my * my * my * color_par.mx_x0y3) +
		(mx * mx * my * color_par.mx_x2y1) + (my * my * mx * color_par.mx_x1y2) +
		(mx * mx * color_par.mx_x2y0) + (my * my * color_par.mx_x0y2) + (mx * my * color_par.mx_x1y1) +
		(mx * color_par.mx_x1y0) + (my * color_par.mx_x0y1) + (color_par.mx_x0y0);

	float wy =
		(mx * mx * mx * color_par.my_x3y0) + (my * my * my * color_par.my_x0y3) +
		(mx * mx * my * color_par.my_x2y1) + (my * my * mx * color_par.my_x1y2) +
		(mx * mx * color_par.my_x2y0) + (my * my * color_par.my_x0y2) + (mx * my * color_par.my_x1y1) +
		(mx * color_par.my_x1y0) + (my * color_par.my_x0y1) + (color_par.my_x0y0);

	float rx = (wx / (color_par.fx * color_q)) - (color_par.shift_m / color_par.shift_d);
	float ry = (wy / color_q) + color_par.cy;
	float cx = (rx + (color_par.shift_m / z)) * color_par.fx + color_par.cx;
	return cv::Vec2f(cx, ry);
}


cv::Vec3f kinect_reprojection::backproject_depth(cv::Vec2f undistorted, float z) const {
	const auto& depth_par = intrinsics_.ir;
	float x = ((undistorted[0] + 0.5f - depth_par.cx) / depth_par.fx) * z;
	float y = ((undistorted[1] + 0.5f - depth_par.cy) / depth_par.fy) * z;
	return cv::Vec3f(x, y, z);
}


}

