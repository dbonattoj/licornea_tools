#include <opencv2/opencv.hpp>
#include "lib/reprojection/kinect_reprojection.h"
#include "../lib/point.h"
#include "../lib/ply_exporter.h"
#include "../lib/image_io.h"
#include "lib/kinect_intrinsics.h"
#include "lib/common.h"
#include <cstdlib>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <string>
#include <cmath>

using namespace tlz;

std::vector<point_xyz> generate_point_cloud(const cv::Mat_<ushort>& in, const kinect_intrinsic_parameters& intrinsics) {
	kinect_reprojection reproj(intrinsics);
	
	std::vector<point_xyz> points;
	points.reserve(depth_width * depth_height);

	for(int dy = 0; dy < depth_height; ++dy) for(int dx = 0; dx < depth_width; ++dx) {
		ushort dz = in(dy, dx);
		if(dz == 0) continue;
		
		vec2 distorted_depth_position(dx, dy);
		vec2 undistorted_depth_position = reproj.undistort_depth(distorted_depth_position);
		
		vec3 point = reproj.backproject_depth(undistorted_depth_position, dz);
		bool null = std::isnan(point[0]);
									
		if(! null) points.emplace_back(vec3(point[0], point[1], point[2]));
	}
	
	points.shrink_to_fit();
	return points;
}

[[noreturn]] void usage_fail() {
	std::cout << "usage: depth_point_cloud input_depth.png output_point_cloud.ply intrinsics.json" << std::endl;
	std::exit(1);
}


int main(int argc, const char* argv[]) {
	if(argc <= 3) usage_fail();
	std::string input_filename = argv[1];
	std::string output_filename = argv[2];
	std::string intrinsics_filename = argv[3];
	
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
	points = generate_point_cloud(in_depth, intrinsics);

	std::cout << "saving output point cloud" << std::endl;
	if(output_filename != "-") {
		ply_exporter exp(output_filename, false, true);
		exp.write(points.begin(), points.end());
		exp.close();
	}
	
	std::cout << "done" << std::endl;
}
