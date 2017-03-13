#include <opencv2/opencv.hpp>
#include <json.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include "../lib/json.h"

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: undistort in_image.png out_image.json intrinsic.json distortion.json\n";
	std::cout << std::endl;
	std::exit(1);
}

int main(int argc, const char* argv[]) {
	if(argc <= 4) usage_fail();
	std::string in_image_filename = argv[1];
	std::string out_image_filename = argv[2];
	std::string in_intrinsic_filename = argv[3];
	std::string in_distortion_filename = argv[4];
	
	cv::Mat in_image = cv::imread(in_image_filename);
	cv::Mat out_image;
	cv::Mat_<double> intrinsic = decode_mat(import_json_file(in_intrinsic_filename));
	cv::Mat_<double> distortion = decode_mat(import_json_file(in_distortion_filename));
	
	cv::undistort(in_image, out_image, intrinsic, distortion);
	
	cv::imwrite(out_image_filename, out_image);
}

