#include <opencv2/opencv.hpp>
#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/registration.h>
#include "../lib/point.h"
#include "../lib/ply_importer.h"
#include "../lib/eigen.h"
#include "../lib/camera.h"
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

void do_point_cloud_reprojection(
	const std::vector<point_xyz>& in_pc,
	cv::Mat_<ushort>& out,
	cv::Mat_<uchar>& out_mask,
	const Eigen_mat3& camera_mat,
	const Eigen_affine3& transformation
) {
	out.setTo(0);
	out_mask.setTo(0);
	for(const point_xyz& pt : in_pc) {
		Eigen_vec3 position = transformation * pt.position();
		Eigen_vec3 im_pt = camera_mat * position;
		im_pt /= im_pt[2];
		int x = im_pt[0], y = im_pt[1];
		if(x < 0 || x >= texture_width || y < 0 || y >= texture_height) continue;
		
		cv::circle(out, cv::Point(x, y), 5, pt.position()[2], -1);
	}
}

[[noreturn]] void usage_fail() {
	std::cout << "usage: point_cloud_reprojection input_point_cloud.ply output.png output_mask.png intrinsics.json [cameras.json in_camera out_camera]" << std::endl;
	std::exit(EXIT_FAILURE);
}

int main(int argc, const char* argv[]) {
	if(argc <= 4) usage_fail();
	std::string input_point_cloud_filename = argv[1];
	std::string output_filename = argv[2];
	std::string output_mask_filename = argv[3];
	std::string intrinsics_filename = argv[4];
	std::string in_cameras_filename;
	std::string in_camera_name;
	std::string out_camera_name;
	if(argc > 5) {
		if(argc <= 7) usage_fail();
		in_cameras_filename = argv[5];
		in_camera_name = argv[6];
		out_camera_name = argv[7];
	}
	
	std::cout << "reading intrinsics" << std::endl;
	kinect_intrinsic_parameters intrinsics;
	{
		std::ifstream str(intrinsics_filename);
		intrinsics = import_intrinsic_parameters(str);
	}
	Eigen_mat3 camera_mat; camera_mat <<
		intrinsics.color.fx, 0.0, intrinsics.color.cx,
		0.0, intrinsics.color.fy, intrinsics.color.cy,
		0.0, 0.0, 1.0;


	Eigen_affine3 transformation = Eigen_affine3::Identity();
	if(! in_cameras_filename.empty()) {
		std::cout << "getting affine transformation" << std::endl;
		auto cameras = cameras_map(read_cameras_file(in_cameras_filename));
		Eigen_mat4 in_extrinsic = cameras.at(in_camera_name).extrinsic;
		Eigen_mat4 out_extrinsic = cameras.at(out_camera_name).extrinsic;
		transformation = Eigen_affine3(out_extrinsic) * Eigen_affine3(in_extrinsic).inverse();
	}
	
	std::cout << "reading input point cloud" << std::endl;
	std::vector<point_xyz> input_point_cloud;
	{
		ply_importer imp(input_point_cloud_filename);
		input_point_cloud.assign(imp.size(), point_xyz());
		imp.read(input_point_cloud.data(), input_point_cloud.size());
	}
		

	std::cout << "preforming point cloud to depth projection" << std::endl;
	cv::Mat_<ushort> out_depth(texture_height, texture_width);
	cv::Mat_<uchar> out_mask(texture_height, texture_width);
	do_point_cloud_reprojection(input_point_cloud, out_depth, out_mask, camera_mat, transformation);
	
	std::cout << "saving output depth map+mask" << std::endl;
	cv::flip(out_depth, out_depth, 1);
	cv::flip(out_mask, out_mask, 1);
	save_depth(output_filename.c_str(), out_depth);
	cv::imwrite(output_mask_filename.c_str(), out_mask);
	
	std::cout << "done" << std::endl;
}
