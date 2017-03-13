/*
#include <opencv2/opencv.hpp>
#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/registration.h>
#include <mf/point_cloud/point.h>
#include <mf/io/ply_importer.h>
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


int main(int argc, const char* argv[]) {
	if(argc <= 4) {
		std::cout << "usage: " << argv[0] << " input_point_cloud.ply output.png output_mask.png intrinsics.json" << std::endl;
		return EXIT_FAILURE;
	}
	const char* input_point_cloud_filename = argv[1];
	const char* output_reprojected_depth_filename = argv[2];
	const char* output_mask_filename = argv[3];
	const char* intrinsics_filename = argv[4];
	
	std::cout << "reading intrinsics" << std::endl;
	kinect_intrinsic_parameters intrinsics;
	{
		std::ifstream str(intrinsics_filename);
		intrinsics = import_intrinsic_parameters(str);
	}
	
	std::cout << "reading input point cloud" << std::endl;
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
*/
