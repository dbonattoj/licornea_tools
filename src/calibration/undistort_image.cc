#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include "../lib/args.h"
#include "../lib/json.h"
#include "../lib/image_io.h"
#include "../lib/opencv.h"
#include "../lib/intrinsics.h"

using namespace tlz;


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "in_image.png out_image.json intrinsics.json texture/depth");
	std::string in_image_filename = in_filename_arg();
	std::string out_image_filename = out_filename_arg();
	intrinsics intr = intrinsics_arg();
	std::string mode = enum_arg({ "texture", "depth" });
		
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

	}
}

