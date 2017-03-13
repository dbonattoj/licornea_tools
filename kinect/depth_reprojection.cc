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

using namespace tlz;

void do_depth_reprojection_map(const cv::Mat_<ushort>& in, cv::Mat_<ushort>& out, cv::Mat_<uchar>& out_mask, const kinect_intrinsic_parameters& intrinsics) {
	using namespace libfreenect2;	

	cv::Mat depth_mat(depth_height, depth_width, CV_32FC1);
	in.convertTo(depth_mat, CV_32FC1);
	Frame depth_frame(depth_width, depth_height, 4, depth_mat.data);
	
	Frame color_frame(texture_width, texture_height, 4);
	Frame depth_undistorted_frame(depth_width, depth_height, 4);
	Frame registered_color_frame(depth_width, depth_height, 4);
	
	cv::Mat bigdepth_mat(1082, 1920, CV_32FC1);
	Frame bigdepth_frame(1920, 1082, 4, bigdepth_mat.data);
	
	Registration reg(intrinsics.ir, intrinsics.color);
	
	std::cout << "Registration::apply..." << std::endl;
	reg.apply(
		&color_frame,
		&depth_frame,
		&depth_undistorted_frame,
		&registered_color_frame,
		true,
		&bigdepth_frame
	);
	std::cout << "Registration::apply." << std::endl;
	
	out = bigdepth_mat.rowRange(1, 1081);
}


void do_depth_reprojection_splat(const cv::Mat_<ushort>& in, cv::Mat_<ushort>& out, cv::Mat_<uchar>& out_mask, const kinect_intrinsic_parameters& intrinsics) {
	using namespace libfreenect2;
	
	out.setTo(0);
	out_mask.setTo(0);
	
	Registration reg(intrinsics.ir, intrinsics.color);
	
	int splat_radius = 5;
	
	for(int dy = 0; dy < depth_height; ++dy) for(int dx = 0; dx < depth_width; ++dx) {
		ushort dz = in(dy, dx);
		if(dz == 0) continue;
						
		float cx, cy;
		reg.apply(dx, dy, dz, cx, cy);
		
		unsigned cx_int = cx, cy_int = cy;
		if(cx_int >= texture_width || cy_int >= texture_height) continue;
		
		cv::circle(out, cv::Point(cx, cy), splat_radius, dz, -1);
		cv::circle(out_mask, cv::Point(cx, cy), splat_radius, 0xff, -1);
	}
}


int main(int argc, const char* argv[]) {
	if(argc <= 5) {
		std::cout << "usage: " << argv[0] << " input.png output.png output_mask.png intrinsics.json splat/map" << std::endl;
		return EXIT_FAILURE;
	}
	const char* input_filename = argv[1];
	const char* output_filename = argv[2];
	const char* output_mask_filename = argv[3];
	const char* intrinsics_filename = argv[4];
	std::string mode = argv[5];
	
	std::cout << "reading intrinsics" << std::endl;
	kinect_intrinsic_parameters intrinsics;
	{
		std::ifstream str(intrinsics_filename);
		intrinsics = import_intrinsic_parameters(str);
	}
	
	std::cout << "reading input depth map" << std::endl;
	cv::Mat_<ushort> in_depth = load_depth(input_filename);
	cv::flip(in_depth, in_depth, 1);
		
	std::cout << "preforming depth mapping" << std::endl;
	cv::Mat_<ushort> out_depth(texture_height, texture_width);
	cv::Mat_<uchar> out_mask(texture_height, texture_width);
	if(mode == "splat") do_depth_reprojection_splat(in_depth, out_depth, out_mask, intrinsics);
	else if(mode == "map") do_depth_reprojection_map(in_depth, out_depth, out_mask, intrinsics);
	else throw std::runtime_error("unknown mode");
	
	std::cout << "saving output depth map+mask" << std::endl;
	cv::flip(out_depth, out_depth, 1);
	cv::flip(out_mask, out_mask, 1);
	save_depth(output_filename, out_depth);
	cv::imwrite(output_mask_filename, out_mask);
	
	std::cout << "done" << std::endl;
}
