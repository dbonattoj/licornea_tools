#include <opencv2/opencv.hpp>
#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/registration.h>
#include "lib/kinect_intrinsics.h"
#include <cstdlib>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <fstream>

using ushort = std::uint16_t;

constexpr std::size_t input_width = 512;
constexpr std::size_t input_height = 424;
constexpr std::size_t output_width = 1920;
constexpr std::size_t output_height = 1080;

cv::Mat_<ushort> load_depth(const char* filename) {
	cv::Mat mat = cv::imread(filename, CV_LOAD_IMAGE_ANYDEPTH);
	if(mat.depth() != CV_16U) throw std::runtime_error("input depth map: must be 16 bit");
	if(mat.rows != input_height || mat.cols != input_width) throw std::runtime_error("input depth map: wrong size");
	cv::Mat_<ushort> mat_ = mat;
	return mat_;
}


void save_depth(const char* filename, const cv::Mat_<ushort>& depth) {
	std::vector<int> params = { CV_IMWRITE_PNG_COMPRESSION, 0 };
	cv::imwrite(filename, depth, params);
}


void do_depth_reprojection(const cv::Mat_<ushort>& in, cv::Mat_<ushort>& out, cv::Mat_<uchar>& out_mask, const kinect_intrinsic_parameters& intrinsics) {
	using namespace libfreenect2;
	
	out.setTo(0);
	out_mask.setTo(0);
	
	Registration reg(intrinsics.ir, intrinsics.color);
	
	int splat_radius = 5;
	
	for(int dy = 0; dy < input_height; ++dy) for(int dx = 0; dx < input_width; ++dx) {
		ushort dz = in(dy, dx);
		if(dz == 0) continue;
						
		float cx, cy;
		reg.apply(dx, dy, -dz, cx, cy);
		
		unsigned cx_int = cx, cy_int = cy;
		if(cx_int >= output_width || cy_int >= output_height) continue;
		
		cv::circle(out, cv::Point(cx, cy), splat_radius, dz, -1);
		cv::circle(out_mask, cv::Point(cx, cy), splat_radius, 0xff, -1);
	}
}


int main(int argc, const char* argv[]) {
	if(argc <= 4) {
		std::cout << "usage: " << argv[0] << " input.png output.png output_mask.png intrinsics.json" << std::endl;
		return EXIT_FAILURE;
	}
	const char* input_filename = argv[1];
	const char* output_filename = argv[2];
	const char* output_mask_filename = argv[3];
	const char* intrinsics_filename = argv[4];
	
	std::cout << "reading intrinsics" << std::endl;
	kinect_intrinsic_parameters intrinsics;
	{
		std::ifstream str(intrinsics_filename);
		intrinsics = import_intrinsic_parameters(str);
	}
	
	std::cout << "reading input depth map" << std::endl;
	cv::Mat_<ushort> in_depth = load_depth(input_filename);
	
	std::cout << "preforming depth mapping" << std::endl;
	cv::Mat_<ushort> out_depth(output_height, output_width);
	cv::Mat_<uchar> out_mask(output_height, output_width);
	do_depth_reprojection(in_depth, out_depth, out_mask, intrinsics);
	
	std::cout << "saving output depth map+mask" << std::endl;
	save_depth(output_filename, out_depth);
	cv::imwrite(output_mask_filename, out_mask);
	
	std::cout << "done" << std::endl;
}
