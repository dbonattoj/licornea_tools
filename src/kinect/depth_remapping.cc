#include "../lib/args.h"
#include "../lib/point.h"
#include "../lib/ply_exporter.h"
#include "../lib/image_io.h"
#include "../lib/opencv.h"
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


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "input.png output.png output_mask.png internal_parameters.json method [=was_flipped]");
	std::string input_filename = in_filename_arg();
	std::string output_filename = out_filename_opt_arg();
	std::string output_mask_filename = out_filename_opt_arg();
	std::string internal_parameters_filename = in_filename_arg();
	std::string method = string_arg();
	bool was_flipped = bool_opt_arg("was_flipped", true);
	
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
	if(!output_filename.empty()) save_depth(output_filename.c_str(), out_depth);
	if(!output_mask_filename.empty()) cv::imwrite(output_mask_filename.c_str(), out_mask);
	
	std::cout << "done" << std::endl;
}
