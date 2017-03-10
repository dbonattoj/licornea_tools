#include <opencv2/opencv.hpp>
#include <json.hpp>
#include <iostream>
#include <string>
#include <cstdlib>
#include <iterator>

#include "../../lib/json.h"
#include "lib/calibration_correspondence.h"

[[noreturn]] void usage_fail() {
	std::cout << "usage: calibrate_distortion {in_correspondences_1.json ...} -- image_width image_height in_intrinsic.json out_distortion.json\n";
	std::cout << std::endl;
	std::exit(1);
}

int main(int argc, const char* argv[]) {
	if(argc <= 1) usage_fail();
	std::vector<std::string> in_correspondences_filenames;
	int argpos = 1;
	for(; argpos < argc; ++argpos) {
		std::string arg = argv[argpos];
		if(arg == "--") break;
		else in_correspondences_filenames.push_back(arg);
	}
	if(argc - argpos - 1 < 4) usage_fail();
	int image_width = std::atoi(argv[argpos + 1]);
	int image_height = std::atoi(argv[argpos + 2]);
	std::string in_intrinsic_filename = argv[argpos + 3];
	std::string out_distortion_filename = argv[argpos + 4];
		
	std::vector<std::vector<cv::Vec3f>> objects_points;
	std::vector<std::vector<cv::Vec2f>> images_points;
	for(const std::string& filename : in_correspondences_filenames) {
		std::vector<calibration_correspondence> cors;
		decode_calibration_correspondences(import_json_file(filename), std::inserter(cors, cors.end()));
		
		std::vector<cv::Vec3f> object_points;
		std::vector<cv::Vec2f> image_points;
		for(const calibration_correspondence& cor : cors) {
			object_points.push_back(cor.object_coordinates);
			image_points.push_back(cor.image_coordinates);
		}
		objects_points.push_back(object_points);
		images_points.push_back(image_points);
	}
	
	cv::Mat_<double> intrinsic = decode_mat(import_json_file(in_intrinsic_filename));
	
	cv::Mat_<double> distortion(5, 1);
		
	int flags = CV_CALIB_USE_INTRINSIC_GUESS;

	std::vector<cv::Mat> rotations, translations;
	double reprojection_error = cv::calibrateCamera(
		objects_points,
		images_points,
		cv::Size(image_width, image_height),
		intrinsic,
		distortion,
		rotations,
		translations,
		flags
	);
	std::cout << "reprojection error: " << reprojection_error << std::endl;
	
	export_json_file(encode_mat(distortion), out_distortion_filename);
}

