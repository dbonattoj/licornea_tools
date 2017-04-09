#include "lib/kinect_intrinsics.h"
#include "lib/reprojection/kinect_reprojection.h"
#include "lib/densify/depth_densify.h"
#include "lib/common.h"
#include "lib/texture_io.h"
#include "lib/depth_io.h"
#include "../lib/nd.h"
#include "../lib/opencv.h"
#include <iostream>
#include <fstream>
#include <string>

using namespace tlz;


int main(int argc, const char* argv[]) {
	if(argc <= 8) {
		std::cout << "usage: " << argv[0] << " input_depth.png input_texture.png output_depth.png output_texture.png output_mask.png intrinsics.json out_width out_height" << std::endl;
		return EXIT_FAILURE;
	}
	const char* input_depth_filename = argv[1];
	const char* input_texture_filename = argv[2];
	const char* output_depth_filename = argv[3];
	const char* output_texture_filename = argv[4];
	const char* output_mask_filename = argv[5];
	const char* intrinsics_filename = argv[6];
	int output_width = std::atoi(argv[7]);
	int output_height = std::atoi(argv[8]);
	
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
		
	kinect_reprojection reproj(intrinsics);
	cv::Size reprojected_size(output_width, output_height);

	std::cout << "doing texture reprojection" << std::endl;
	cv::Mat_<cv::Vec3b> out_texture = reproj.reproject_color(in_texture, reprojected_size);
	
	for(float p = 1000.0; p < 2000.0; p += 500.0) {
		std::cout << "doing depth reprojection" << std::endl;
		std::vector<Eigen_vec3> reprojected_depth_samples = reproj.reproject_depth(in_depth, reprojected_size, p);
	
		std::cout << "doing depth densification" << std::endl;
		cv::Mat_<float> out_densify_depth(reprojected_size);
		cv::Mat_<uchar> out_densify_mask(reprojected_size);
		make_depth_densify("mine")->densify(reprojected_depth_samples, out_densify_depth, out_densify_mask);
	
		std::cout << "saving output texture+depth+mask" << std::endl;
		cv::Mat_<ushort> out_depth = out_densify_depth*20.0;
	
		cv::flip(out_depth, out_depth, 1);
		save_depth(("depth_"+std::to_string(p)+".png").c_str(), out_depth);
	}
	/*
	std::cout << "doing depth reprojection" << std::endl;
	std::vector<Eigen_vec3> reprojected_depth_samples = reproj.reproject_depth(in_depth, reprojected_size, 0.0001);
	
	std::cout << "doing depth densification" << std::endl;
	cv::Mat_<float> out_densify_depth(reprojected_size);
	cv::Mat_<uchar> out_densify_mask(reprojected_size);
	make_depth_densify("mine")->densify(reprojected_depth_samples, out_densify_depth, out_densify_mask);
	
	std::cout << "saving output texture+depth+mask" << std::endl;
	cv::Mat_<ushort> out_depth = out_densify_depth*20.0;
	
	cv::flip(out_texture, out_texture, 1);
	cv::flip(out_depth, out_depth, 1);
	//cv::flip(out_mask, out_mask, 1);
	save_texture(output_texture_filename, out_texture);
	save_depth(output_depth_filename, out_depth);
	//cv::imwrite(output_mask_filename, out_mask);
*/

	
	std::cout << "done" << std::endl;
}
