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

std::vector<point_xyz> generate_point_cloud(const cv::Mat_<ushort>& in, const kinect_intrinsic_parameters& intrinsics) {
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


int main(int argc, const char* argv[]) {
	if(argc <= 3) {
		std::cout << "usage: " << argv[0] << " input_depth.png output_point_cloud.ply intrinsics.json" << std::endl;
		return EXIT_FAILURE;
	}
	const char* input_filename = argv[1];
	const char* output_filename = argv[2];
	const char* intrinsics_filename = argv[3];
	
	std::cout << "reading intrinsics" << std::endl;
	kinect_intrinsic_parameters intrinsics;
	{
		std::ifstream str(intrinsics_filename);
		intrinsics = import_intrinsic_parameters(str);
	}
	
	std::cout << "reading input depth map" << std::endl;
	cv::Mat_<ushort> in_depth = load_depth(input_filename);
	cv::flip(in_depth, in_depth, 1);
	
	std::cout << "making point cloud" << std::endl;
	auto points = generate_point_cloud(in_depth, intrinsics);

	std::cout << "saving output point cloud" << std::endl;
	ply_exporter exp(output_filename, false, true);
	exp.write(points.begin(), points.end());
	exp.close();
		
	std::cout << "done" << std::endl;
}
