/*
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


cv::Vec2f kinect_reprojection::distort_depth_(cv::Vec2f undistorted) const {
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


cv::Vec2f kinect_reprojection::undistort_depth_(cv::Vec2f distorted) const {
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


cv::Vec2f kinect_reprojection::reproject_depth_to_color_(cv::Vec2f undistorted, float z) const {
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

	
cv::Mat_<cv::Vec3b> kinect_reprojection::reproject_color(
	const cv::Mat_<cv::Vec3b>& color,
	cv::Size reprojected_size
) const {
	cv::Mat_<cv::Vec3b> reprojected_color(reprojected_size);
	
	if(color.size() != cv::Size(texture_width, texture_height)) throw std::runtime_error("incorrect input texture size");
	
	for(int reproj_y = 0; reproj_y < reprojected_size.height; ++reproj_y)
	for(int reproj_x = 0; reproj_x < reprojected_size.width; ++reproj_x) {						
		cv::Vec2f reproj_pos(reproj_x, reproj_y);
		cv::Vec2f undistorted_depth_pos(
			reproj_pos[0] * depth_width / reprojected_size.width,
			reproj_pos[1] * depth_height / reprojected_size.height
		);
				
		cv::Vec2f color_pos = reproject_depth_to_color_(undistorted_depth_pos, 1000.0);
				
		if(color_pos[0] >= 0 && color_pos[0] < texture_width
		&& color_pos[1] >= 0 && color_pos[1] < texture_height) {
			cv::Point color_coord(color_pos[0], color_pos[1]);
	
			reprojected_color(reproj_y, reproj_x) = color(color_coord);
		} else {
			reprojected_color(reproj_y, reproj_x) = cv::Vec3b(0,0,0);
		}
	}
	
	return reprojected_color;
}


std::tuple<cv::Mat_<cv::Vec3b>, std::vector<Eigen_vec3>> reproject_dual(
	const cv::Mat_<cv::Vec3b>& color,
	const cv::Mat_<float>& distorted_depth,
	cv::Size reprojected_size
) const {
	const auto& depth_par = intrinsics_.ir;
	const auto& color_par = intrinsics_.color;

	cv::Mat_<cv::Vec3b> reprojected_color(reprojected_size);

	std::vector<Eigen_vec3> reprojected_depth_samples;
	reprojected_depth_samples.reserve(depth_width * depth_height);

	if(color.size() != cv::Size(texture_width, texture_height)) throw std::runtime_error("incorrect input texture size");
	if(distorted_depth.size() != cv::Size(depth_width, depth_height)) throw std::runtime_error("incorrect input depth size");
	
	for(int reproj_y = 0; reproj_y < reprojected_size.height; ++reproj_y)
	for(int reproj_x = 0; reproj_x < reprojected_size.width; ++reproj_x) {						
		cv::Vec2f reproj_pos(reproj_x, reproj_y);
		cv::Vec2f undistorted_depth_pos(
			reproj_pos[0] * depth_width / reprojected_size.width,
			reproj_pos[1] * depth_height / reprojected_size.height
		);
		
		cv::Vec2f distorted_depth_pos = distort_depth_(undistorted_depth_pos);
				
		cv::Vec2f color_pos = reproject_depth_to_color_(undistorted_depth_pos, 1000.0);
				
		if(color_pos[0] >= 0 && color_pos[0] < texture_width
		&& color_pos[1] >= 0 && color_pos[1] < texture_height) {
			cv::Point color_coord(color_pos[0], color_pos[1]);
	
			reprojected_color(reproj_y, reproj_x) = color(color_coord);
		} else {
			reprojected_color(reproj_y, reproj_x) = cv::Vec3b(0,0,0);
		}
	}
	
	
	return std::make_tuple(reprojected_color, reprojected_depth_samples);
}



std::vector<Eigen_vec3> kinect_reprojection::reproject_depth(
	const cv::Mat_<float>& distorted_depth,
	cv::Size reprojected_size,
	float p
) const {
	const auto& depth_par = intrinsics_.ir;
	const auto& color_par = intrinsics_.color;
	
	std::vector<Eigen_vec3> reprojected_depth_samples;
	reprojected_depth_samples.reserve(depth_width * depth_height);

	if(distorted_depth.size() != cv::Size(depth_width, depth_height)) throw std::runtime_error("incorrect input depth size");
		
	for(int distorted_y = 0; distorted_y < depth_height; ++distorted_y)
	for(int distorted_x = 0; distorted_x < depth_width; ++distorted_x) {	
		float depth = distorted_depth(distorted_y, distorted_x);
		if(depth == 0.0) continue;
		
		cv::Vec2f distorted_pos(distorted_y, distorted_x);
	
		cv::Vec2f undistorted_pos = undistort_depth_(distorted_pos);
		
		cv::Vec2f flat_pos;
		flat_pos[0] = undistorted_pos[0];
		flat_pos[1] = undistorted_pos[1];
				
		cv::Vec2f reproj_pos(
			flat_pos[0] * reprojected_size.width / depth_width,
			flat_pos[1] * reprojected_size.height / depth_height
		);
		
		reprojected_depth_samples.emplace_back(reproj_pos[1], reproj_pos[0], depth);
	}
	
	return reprojected_depth_samples;
}


/*
void kinect_reprojection::reproject_both(
	const cv::Mat_<float>& distorted_depth,
	const cv::Mat_<cv::Vec3b>& color,
	cv::Mat_<float>& reprojected_depth,
	cv::Mat_<cv::Vec3b>& reprojected_color
) const {	
	if(distorted_depth.size() != cv::Size(depth_width, depth_height)) throw std::runtime_error("incorrect input distorted depth size");
	if(color.size() != cv::Size(texture_width, texture_height)) throw std::runtime_error("incorrect input texture size");
	if(reprojected_depth.size() != reprojected_color.size()) throw std::runtime_error("output texture&depth sizes must be same");
	cv::Size reprojected_size = reprojected_depth.size();
	
	for(int reproj_y = 0; reproj_y < reprojected_size.height; ++reproj_y)
	for(int reproj_x = 0; reproj_x < reprojected_size.width; ++reproj_x) {
		cv::Point reproj_coord(reproj_x, reproj_y);
		
		float& reproj_depth = reprojected_depth(reproj_coord);
		cv::Vec3b& reproj_color = reprojected_color(reproj_coord);
		
		reproj_depth = 0.0;
		reproj_color = cv::Vec3b(0,0,0);
				
		cv::Vec2f reproj_pos(reproj_x, reproj_y);
		cv::Vec2f undistorted_depth_pos(
			reproj_pos[0] * depth_width / reprojected_size.width,
			reproj_pos[1] * depth_height / reprojected_size.height
		);
		
		cv::Vec2f distorted_depth_pos = distort_depth(undistorted_depth_pos);
		
		float depth = 0.0;
		if(distorted_depth_pos[0] >= 0 && distorted_depth_pos[0] < depth_width
		&& distorted_depth_pos[1] >= 0 && distorted_depth_pos[1] < depth_height) {
			cv::Point distorted_depth_coord(distorted_depth_pos[0], distorted_depth_pos[1]);
			depth = distorted_depth(distorted_depth_coord);
			//std::cout << depth << std::endl;
		}
		
		if(depth == 0.0) continue;
		
		float reprojection_depth = (depth != 0.0 ? depth : 1600.0);
		cv::Vec2f color_pos = reproject_depth_to_color(undistorted_depth_pos, reprojection_depth);
		
		//std::cout << color_pos << std::endl;
		
		if(color_pos[0] >= 0 && color_pos[0] < texture_width
		&& color_pos[1] >= 0 && color_pos[1] < texture_height) {
			cv::Point color_coord(color_pos[0], color_pos[1]);
	
			reproj_color = color(color_coord);
			reproj_depth = 20.0*depth;
		}
	}
}
* /

}
*/
