#include <opencv2/opencv.hpp>
#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/registration.h>
#include "../lib/point.h"
#include "../lib/ply_exporter.h"
#include "lib/kinect_intrinsics.h"
#include "lib/common.h"
#include "lib/depth_io.h"
#include "lib/texture_io.h"
#include <cstdlib>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <string>
#include <cmath>

using namespace tlz;

std::vector<point_full> generate_point_cloud(
	const cv::Mat_<ushort>& in_depth,
	const cv::Mat_<cv::Vec3b>& in_texture,
	const kinect_intrinsic_parameters& intrinsics)
{
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
	
	std::cout << "reprojecting texture & undistorting depth" << std::endl;
	reg.apply(
		&color_frame,
		&depth_frame,
		&depth_undistorted_frame,
		&registered_color_frame,
		false
	);

	std::vector<point_full> points;
	points.reserve(depth_width * depth_height);

	std::cout << "backprojecting point cloud" << std::endl;
	for(int row = 0; row < depth_height; ++row) for(int col = 0; col < depth_width; ++col) {
		Eigen_vec3 position;
		float rgb_f;
		
		reg.getPointXYZRGB(&depth_undistorted_frame, &registered_color_frame, row, col, position[0], position[1], position[2], rgb_f);
		bool null = std::isnan(position[0]);
		if(! null) {
			position *= 1000.0f; // from meters to mm
			const std::uint8_t* rgb_p = reinterpret_cast<const std::uint8_t*>(&rgb_f);	
			rgb_color rgb(rgb_p[0], rgb_p[1], rgb_p[2]);
			points.emplace_back(position, rgb);
		}
	}
	
	points.shrink_to_fit();
	
	return points;
}


int main(int argc, const char* argv[]) {
	if(argc <= 4) {
		std::cout << "usage: " << argv[0] << " input_texture.png input_depth.png output_point_cloud.ply intrinsics.json" << std::endl;
		return EXIT_FAILURE;
	}
	const char* input_texture_filename = argv[1];
	const char* input_depth_filename = argv[2];
	const char* output_filename = argv[3];
	const char* intrinsics_filename = argv[4];
	
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
	
	std::cout << "making point cloud" << std::endl;
	auto points = generate_point_cloud(in_depth, in_texture, intrinsics);

	std::cout << "saving output point cloud" << std::endl;
	ply_exporter exp(output_filename, true, true);
	exp.write(points.begin(), points.end());
	exp.close();
		
	std::cout << "done" << std::endl;
}
