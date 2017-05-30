#include <opencv2/opencv.hpp>
#include "../lib/point.h"
#include "../lib/ply_exporter.h"
#include "../lib/image_io.h"
#include "lib/densify/depth_densify.h"
#include "lib/kinect_internal_parameters.h"
#include "lib/kinect_remapping.h"
#include "lib/common.h"
#include <cstdlib>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <string>

using namespace tlz;


void do_depth_remapping(const cv::Mat_<ushort>& in, cv::Mat_<ushort>& out, cv::Mat_<uchar>& out_mask, const kinect_remapping& remap, const std::string& method) {	
	cv::Mat_<real> in_float = in;
	auto samples = remap.remap_ir_to_color_samples(in_float, in_float);
	cv::Mat_<real> out_float(texture_height, texture_width);
	make_depth_densify(method)->densify(samples, out_float, out_mask);
	out = out_float;
}


[[noreturn]] void usage_fail() {
	std::cout << "usage: depth_reprojection input.png output.png output_mask.png internal_parameters.json method [was_flipped/was_not_flipped]" << std::endl;
	std::exit(EXIT_FAILURE);
}
int main(int argc, const char* argv[]) {
	if(argc <= 5) usage_fail();
	std::string input_filename = argv[1];
	std::string output_filename = argv[2];
	std::string output_mask_filename = argv[3];
	std::string internal_parameters_filename = argv[4];
	std::string method = argv[5];
	bool was_flipped = true;
	if(argc > 6) was_flipped = (std::string(argv[6]) == "was_flipped");
	
	std::cout << "reading parameters" << std::endl;
	kinect_internal_parameters internal_parameters = decode_kinect_internal_parameters(import_json_file(internal_parameters_filename));
	kinect_remapping remap(internal_parameters);
	
	std::cout << "reading input depth map" << std::endl;
	cv::Mat_<ushort> in_depth = load_depth(input_filename.c_str());
	if(was_flipped) cv::flip(in_depth, in_depth, 1);
		
	std::cout << "doing depth densification" << std::endl;
	cv::Mat_<ushort> out_depth(texture_height, texture_width);
	cv::Mat_<uchar> out_mask(texture_height, texture_width);
	do_depth_remapping(in_depth, out_depth, out_mask, remap, method);
	
	std::cout << "saving output depth map+mask" << std::endl;
	if(was_flipped) {
		cv::flip(out_depth, out_depth, 1);
		cv::flip(out_mask, out_mask, 1);
	}
	if(output_filename != "-") save_depth(output_filename.c_str(), out_depth);
	if(output_mask_filename != "-") cv::imwrite(output_mask_filename.c_str(), out_mask);
	
	std::cout << "done" << std::endl;
}
