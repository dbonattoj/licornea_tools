#include "kinect_reprojection.h"
#include "../common.h"
#include "../../../lib/utility/assert.h"

#include <iostream>

namespace tlz {

namespace {
	const real depth_q = 0.01;
	const real color_q = 0.002199;
}

kinect_reprojection::kinect_reprojection(const kinect_intrinsic_parameters& intr) :
	intrinsics_(intr) { }


vec2 kinect_reprojection::distort_depth(vec2 undistorted) const {
	const auto& depth_par = intrinsics_.ir;
	real dx = (undistorted[0] - depth_par.cx) / depth_par.fx;
	real dy = (undistorted[1] - depth_par.cy) / depth_par.fy;
	real dx2 = dx * dx;
	real dy2 = dy * dy;
	real r2 = dx2 + dy2;
	real dxdy2 = 2 * dx * dy;
	real kr = 1 + ((depth_par.k3 * r2 + depth_par.k2) * r2 + depth_par.k1) * r2;
	real out_x = depth_par.fx * (dx * kr + depth_par.p2 * (r2 + 2 * dx2) + depth_par.p1 * dxdy2) + depth_par.cx;
	real out_y = depth_par.fy * (dy * kr + depth_par.p1 * (r2 + 2 * dy2) + depth_par.p2 * dxdy2) + depth_par.cy;
	return vec2(out_x, out_y);
}


vec2 kinect_reprojection::undistort_depth(vec2 distorted) const {
	const auto& depth_par = intrinsics_.ir;
	real dx = distorted[0], dy = distorted[1];
    real ps = (dx * dx) + (dy * dy);
    real qs = ((ps * depth_par.k3 + depth_par.k2) * ps + depth_par.k1) * ps + 1.0;
    for (int i = 0; i < 9; i++) {
        real qd = ps / (qs * qs);
        qs = ((qd * depth_par.k3 + depth_par.k2) * qd + depth_par.k1) * qd + 1.0;
    }
    real mx = dx / qs;
    real my = dy / qs;
	return vec2(mx, my);
}


vec2 kinect_reprojection::reproject_depth_to_color(vec2 undistorted, real z) const {
	const auto& depth_par = intrinsics_.ir;
	const auto& color_par = intrinsics_.color;
	real mx = undistorted[0], my = undistorted[1];
	
	mx = (mx - depth_par.cx) * depth_q;
	my = (my - depth_par.cy) * depth_q;
	
	real wx =
		(mx * mx * mx * color_par.mx_x3y0) + (my * my * my * color_par.mx_x0y3) +
		(mx * mx * my * color_par.mx_x2y1) + (my * my * mx * color_par.mx_x1y2) +
		(mx * mx * color_par.mx_x2y0) + (my * my * color_par.mx_x0y2) + (mx * my * color_par.mx_x1y1) +
		(mx * color_par.mx_x1y0) + (my * color_par.mx_x0y1) + (color_par.mx_x0y0);

	real wy =
		(mx * mx * mx * color_par.my_x3y0) + (my * my * my * color_par.my_x0y3) +
		(mx * mx * my * color_par.my_x2y1) + (my * my * mx * color_par.my_x1y2) +
		(mx * mx * color_par.my_x2y0) + (my * my * color_par.my_x0y2) + (mx * my * color_par.my_x1y1) +
		(mx * color_par.my_x1y0) + (my * color_par.my_x0y1) + (color_par.my_x0y0);

	real rx = (wx / (color_par.fx * color_q)) - (color_par.shift_m / color_par.shift_d);
	real ry = (wy / color_q) + color_par.cy;
	real cx = (rx + (color_par.shift_m / z)) * color_par.fx + color_par.cx;
	return vec2(cx, ry);
}


vec3 kinect_reprojection::backproject_depth(vec2 undistorted, real z) const {
	const auto& depth_par = intrinsics_.ir;
	real x = ((undistorted[0] + 0.5f - depth_par.cx) / depth_par.fx) * z;
	real y = ((undistorted[1] + 0.5f - depth_par.cy) / depth_par.fy) * z;
	return vec3(x, y, z);
}


}

