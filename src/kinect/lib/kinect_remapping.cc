#include "kinect_remapping.h"
#include "common.h"
#include "../../lib/opencv.h"

namespace tlz {

namespace {
	const real depth_q = 0.01;
	const real color_q = 0.002199;
}

kinect_remapping::kinect_remapping(const kinect_internal_parameters& internal) :
	internal_parameters_(internal) { }


vec2 kinect_remapping::distort_ir(vec2 undistorted) const {
	const auto& ir_par = internal_parameters_.ir;
	real dx = (undistorted[0] - ir_par.cx) / ir_par.fx;
	real dy = (undistorted[1] - ir_par.cy) / ir_par.fy;
	real dx2 = dx * dx;
	real dy2 = dy * dy;
	real r2 = dx2 + dy2;
	real dxdy2 = 2 * dx * dy;
	real kr = 1 + ((ir_par.k3 * r2 + ir_par.k2) * r2 + ir_par.k1) * r2;
	real out_x = ir_par.fx * (dx * kr + ir_par.p2 * (r2 + 2 * dx2) + ir_par.p1 * dxdy2) + ir_par.cx;
	real out_y = ir_par.fy * (dy * kr + ir_par.p1 * (r2 + 2 * dy2) + ir_par.p2 * dxdy2) + ir_par.cy;
	return vec2(out_x, out_y);
}


vec2 kinect_remapping::undistort_ir(vec2 distorted_coord) const {
	const auto& ir_par = internal_parameters_.ir;
	mat33 camera_mat(
		ir_par.fx, 0.0, ir_par.cx,
		0.0, ir_par.fy, ir_par.cy,
		0.0, 0.0, 1.0
	);
	std::vector<real> distortion { ir_par.k1, ir_par.k2, ir_par.p1, ir_par.p2, ir_par.k3 };
	std::vector<vec2> distorted { distorted_coord };
	std::vector<vec2> undistorted;
	cv::undistortPoints(distorted, undistorted, camera_mat, distortion, cv::noArray(), camera_mat);
	return undistorted.front();
}


vec2 kinect_remapping::map_ir_to_color(vec2 undistorted, real z) const {
	const auto& ir_par = internal_parameters_.ir;
	const auto& color_par = internal_parameters_.color;
	real mx = undistorted[0], my = undistorted[1];
	
	mx = (mx - ir_par.cx) * depth_q;
	my = (my - ir_par.cy) * depth_q;
	
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

}
