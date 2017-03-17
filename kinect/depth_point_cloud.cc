#include <opencv2/opencv.hpp>
#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/registration.h>
#include "../lib/point.h"
#include "../lib/ply_exporter.h"
#include "lib/kinect_intrinsics.h"
#include "lib/common.h"
#include "lib/depth_io.h"
#include <cstdlib>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <string>
#include <cmath>

using namespace tlz;

std::vector<point_xyz> generate_point_cloud_from_ir(const cv::Mat_<ushort>& in, const kinect_intrinsic_parameters& intrinsics) {
	using namespace libfreenect2;	

	cv::Mat depth_mat(depth_height, depth_width, CV_32FC1);
	in.convertTo(depth_mat, CV_32FC1);
	Frame depth_frame(depth_width, depth_height, 4, depth_mat.data);
	
	Frame depth_undistorted_frame(depth_width, depth_height, 4);
	
	Registration reg(intrinsics.ir, intrinsics.color);
	
	std::cout << "undistorting depth" << std::endl;
	reg.undistortDepth(&depth_frame, &depth_undistorted_frame);
	
	std::vector<point_xyz> points;
	points.reserve(depth_width * depth_height);
	
	std::cout << "backprojecting point cloud" << std::endl;
	for(int row = 0; row < depth_height; ++row) for(int col = 0; col < depth_width; ++col) {
		Eigen_vec3 point;
		reg.getPointXYZ(&depth_undistorted_frame, row, col, point[0], point[1], point[2]);
		bool null = std::isnan(point[0]);
		if(! null) {
			point *= 1000.0f; // from meters to mm
			points.emplace_back(point);
		}
	}
	
	points.shrink_to_fit();
	return points;
}

std::vector<point_xyz> generate_point_cloud_from_color(const cv::Mat_<ushort>& in, const kinect_intrinsic_parameters& intrinsics) {
	using namespace libfreenect2;
		
	Registration reg(intrinsics.ir, intrinsics.color);
		
	cv::Mat_<float> depth_mat(depth_height, depth_width, CV_32FC1);
	in.convertTo(depth_mat, CV_32FC1);
	Frame depth_frame(depth_width, depth_height, 4, depth_mat.data);

	cv::Mat_<float> depth_undistorted(depth_height, depth_width, CV_32FC1);
	Frame depth_undistorted_frame(depth_width, depth_height, 4, depth_undistorted.data);

	reg.undistortDepth(&depth_frame, &depth_undistorted_frame);

	std::vector<point_xyz> points;
	points.reserve(depth_width * depth_height);

	for(int dy = 0; dy < depth_height; ++dy) for(int dx = 0; dx < depth_width; ++dx) {
		ushort dz = depth_undistorted(dy, dx);
		if(dz == 0) continue;
						
		float cx, cy;
		reg.apply(dx, dy, dz, cx, cy);
		
		float z = dz;
		Eigen_vec3 view_pt(
			z * (cx - intrinsics.color.cx) / intrinsics.color.fx,
			z * (cy - intrinsics.color.cy) / intrinsics.color.fy,
			z
		);
		points.emplace_back(view_pt);
	}

	points.shrink_to_fit();
	return points;	
}

[[noreturn]] void usage_fail() {
	std::cout << "usage: depth_point_cloud input_depth.png output_point_cloud.ply intrinsics.json color/ir" << std::endl;
	std::exit(1);
}


int main(int argc, const char* argv[]) {
	if(argc <= 4) usage_fail();
	std::string input_filename = argv[1];
	std::string output_filename = argv[2];
	std::string intrinsics_filename = argv[3];
	std::string sensor = argv[4];
	if(sensor != "color" && sensor != "ir") usage_fail();
	
	
	std::cout << "reading intrinsics" << std::endl;
	kinect_intrinsic_parameters intrinsics;
	{
		std::ifstream str(intrinsics_filename);
		intrinsics = import_intrinsic_parameters(str);
	}
	
	std::cout << "reading input depth map" << std::endl;
	cv::Mat_<ushort> in_depth = load_depth(input_filename.c_str());
	cv::flip(in_depth, in_depth, 1);
	
	
	std::cout << "making point cloud" << std::endl;
	std::vector<point_xyz> points;
	if(sensor == "ir") points = generate_point_cloud_from_ir(in_depth, intrinsics);
	else points = generate_point_cloud_from_color(in_depth, intrinsics);

	std::cout << "saving output point cloud" << std::endl;
	ply_exporter exp(output_filename, false, true);
	exp.write(points.begin(), points.end());
	exp.close();
		
	std::cout << "done" << std::endl;
}
