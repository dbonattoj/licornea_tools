#include <opencv2/opencv.hpp>
#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/registration.h>
#include "lib/kinect_intrinsics.h"
#include "lib/common.h"
#include "lib/texture_io.h"
#include "lib/depth_io.h"
#include <cstdlib>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <memory>

using namespace tlz;

void do_texture_reprojection(
	const cv::Mat_<ushort>& in_depth,
	const cv::Mat_<cv::Vec3b>& in_texture,
	cv::Mat_<ushort>& out_depth,
	cv::Mat_<cv::Vec3b>& out_texture,
	const kinect_intrinsic_parameters& intrinsics
) {
	using namespace libfreenect2;	

	cv::Mat color_mat(texture_height, texture_width, CV_8UC4);
	cv::cvtColor(in_texture, color_mat, CV_BGR2BGRA);
	Frame color_frame(texture_width, texture_height, 4, color_mat.data);
	
	cv::Mat depth_mat(depth_height, depth_width, CV_32FC1);
	in_depth.convertTo(depth_mat, CV_32FC1);
	Frame depth_frame(depth_width, depth_height, 4, depth_mat.data);
	
	cv::Mat depth_undistorted_mat(depth_height, depth_width, CV_32FC1);
	Frame depth_undistorted_frame(depth_width, depth_height, 4, depth_undistorted_mat.data);
	
	cv::Mat registered_color_mat(depth_height, depth_width, CV_8UC4);
	Frame registered_color_frame(depth_width, depth_height, 4, registered_color_mat.data);
		
	Registration reg(intrinsics.ir, intrinsics.color);
	
	reg.apply(
		&color_frame,
		&depth_frame,
		&depth_undistorted_frame,
		&registered_color_frame,
		false
	);

	cv::cvtColor(registered_color_mat, out_texture, CV_BGRA2BGR);
	out_depth = depth_undistorted_mat;
}


int main(int argc, const char* argv[]) {
	if(argc <= 6) {
		std::cout << "usage: " << argv[0] << " input_depth.png input_texture.png output_depth.png output_texture.png output_mask.png intrinsics.json [inpaint_radius ns/telea]" << std::endl;
		return EXIT_FAILURE;
	}
	const char* input_depth_filename = argv[1];
	const char* input_texture_filename = argv[2];
	const char* output_depth_filename = argv[3];
	const char* output_texture_filename = argv[4];
	const char* output_mask_filename = argv[5];
	const char* intrinsics_filename = argv[6];
	int inpaint_radius = 0;
	std::string inpaint_method = "ns";
	if(argc > 7) inpaint_radius = std::atoi(argv[7]);
	if(argc > 8) inpaint_method = argv[8];
	
	cv::Vec3b black(0, 0, 0);

	std::cout << "reading intrinsics" << std::endl;
	kinect_intrinsic_parameters intrinsics;
	{
		std::ifstream str(intrinsics_filename);
		intrinsics = import_intrinsic_parameters(str);
	}
	
	std::cout << "reading input depth map" << std::endl;
	cv::Mat_<ushort> in_depth = load_depth(input_depth_filename);
	cv::flip(in_depth, in_depth, 1);
	
	std::cout << "reading input texture" << std::endl;
	cv::Mat_<cv::Vec3b> in_texture = load_texture(input_texture_filename);
	cv::flip(in_texture, in_texture, 1);
		
	std::cout << "preforming texture mapping" << std::endl;
	cv::Mat_<ushort> out_depth(depth_height, depth_width);
	cv::Mat_<cv::Vec3b> out_texture(depth_height, depth_width);
	do_texture_reprojection(in_depth, in_texture, out_depth, out_texture, intrinsics);
	
	if(inpaint_radius > 0) {
		std::cout << "inpainting" << std::endl;

		cv::Mat_<uchar> texture_holes(depth_height, depth_width);
		cv::inRange(out_texture, black, black, texture_holes);

		int flags = (inpaint_method == "telea" ? cv::INPAINT_TELEA : cv::INPAINT_NS);
		cv::Mat_<cv::Vec3b> out_texture_inpainted(depth_height, depth_width);
		cv::inpaint(out_texture, texture_holes, out_texture_inpainted, inpaint_radius, flags);
		out_texture = out_texture_inpainted;
	}
	
	std::cout << "determining mask" << std::endl;
	cv::Mat_<uchar> out_mask(depth_height, depth_width);
	cv::Mat_<uchar> texture_holes(depth_height, depth_width);
	cv::inRange(out_texture, black, black, texture_holes);
	
	std::cout << "saving output texture+depth+mask" << std::endl;
	cv::flip(out_texture, out_texture, 1);
	cv::flip(out_depth, out_depth, 1);
	cv::flip(out_mask, out_mask, 1);
	save_texture(output_texture_filename, out_texture);
	save_depth(output_depth_filename, out_depth);
	cv::imwrite(output_mask_filename, out_mask);
	
	std::cout << "done" << std::endl;
}
