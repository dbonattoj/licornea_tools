#include <opencv2/opencv.hpp>
#include "../lib/point.h"
#include "../lib/ply_exporter.h"
#include "lib/reprojection/kinect_reprojection.h"
#include "lib/densify/depth_densify.h"
#include "lib/kinect_intrinsics.h"
#include "lib/common.h"
#include "lib/depth_io.h"
#include <cstdlib>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <string>

using namespace tlz;


void do_depth_reprojection(const cv::Mat_<ushort>& in, cv::Mat_<ushort>& out, cv::Mat_<uchar>& out_mask, const kinect_intrinsic_parameters& intrinsics, const std::string& method) {
	kinect_reprojection reproj(intrinsics);
	
	std::vector<Eigen_vec3> samples;
	for(int dy = 0; dy < depth_height; ++dy) for(int dx = 0; dx < depth_width; ++dx) {
		ushort dz = in(dy, dx);
		if(dz == 0) continue;
		
		cv::Vec2f distorted_depth_position(dx, dy);
		cv::Vec2f undistorted_depth_position = reproj.undistort_depth(distorted_depth_position);
		cv::Vec2f color_position = reproj.reproject_depth_to_color(undistorted_depth_position, dz);
									
		samples.emplace_back(color_position[0], color_position[1], dz);
	}
	
	cv::Mat_<float> densify_out(texture_height, texture_width);
	make_depth_densify(method)->densify(samples, densify_out, out_mask);
	out = densify_out;
}


int main(int argc, const char* argv[]) {
	if(argc <= 5) {
		std::cout << "usage: " << argv[0] << " input.png output.png output_mask.png intrinsics.json method" << std::endl;
		return EXIT_FAILURE;
	}
	const char* input_filename = argv[1];
	const char* output_filename = argv[2];
	const char* output_mask_filename = argv[3];
	const char* intrinsics_filename = argv[4];
	std::string method = argv[5];
	
	std::cout << "reading intrinsics" << std::endl;
	kinect_intrinsic_parameters intrinsics;
	{
		std::ifstream str(intrinsics_filename);
		intrinsics = import_intrinsic_parameters(str);
	}
	
	std::cout << "reading input depth map" << std::endl;
	cv::Mat_<ushort> in_depth = load_depth(input_filename);
	cv::flip(in_depth, in_depth, 1);
		
	std::cout << "doing depth densification" << std::endl;
	cv::Mat_<ushort> out_depth(texture_height, texture_width);
	cv::Mat_<uchar> out_mask(texture_height, texture_width);
	do_depth_reprojection(in_depth, out_depth, out_mask, intrinsics, method);
	
	std::cout << "saving output depth map+mask" << std::endl;
	cv::flip(out_depth, out_depth, 1);
	cv::flip(out_mask, out_mask, 1);
	save_depth(output_filename, out_depth);
	cv::imwrite(output_mask_filename, out_mask);
	
	std::cout << "done" << std::endl;
}
