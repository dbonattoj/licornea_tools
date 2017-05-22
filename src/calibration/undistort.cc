#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include "../lib/json.h"
#include "../lib/image_io.h"
#include "../lib/opencv.h"
#include "../lib/intrinsics.h"

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: undistort in_image.png out_image.json intrinsics.json texture/depth\n";
	std::cout << std::endl;
	std::exit(1);
}

int main(int argc, const char* argv[]) {
	if(argc <= 4) usage_fail();
	std::string in_image_filename = argv[1];
	std::string out_image_filename = argv[2];
	std::string in_intrinsics_filename = argv[3];
	std::string mode = argv[4];
	
	intrinsics intr = decode_intrinsics(import_json_file(in_intrinsics_filename));
	
	if(mode == "texture") {
		cv::Mat_<cv::Vec3b> in_image = load_texture(in_image_filename);
		cv::Mat_<cv::Vec3b> out_image;
		cv::undistort(in_image, out_image, intr.K, intr.distortion.cv_coeffs());
		save_texture(out_image_filename, out_image);
		
	} else if(mode == "depth") {
		cv::Mat_<ushort> in_image = load_depth(in_image_filename);
		cv::Mat_<ushort> out_image;
		cv::Mat map1, map2;
		cv::initUndistortRectifyMap(intr.K, intr.distortion.cv_coeffs(), cv::Mat::eye(3, 3, CV_32F), intr.K, in_image.size(), CV_32FC1, map1, map2);
		cv::remap(in_image, out_image, map1, map2, cv::INTER_NEAREST);
		save_depth(out_image_filename, out_image);
		
	} else {
		throw std::runtime_error("mode must be texture or depth");
	}
}

