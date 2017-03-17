#include <opencv2/opencv.hpp>
#include <json.hpp>
#include <iostream>
#include <string>
#include <cstdlib>
#include <iterator>

#include "../lib/json.h"
#include "lib/calibration_correspondence.h"

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: calibrate_extrinsic in_correspondences.json in_intrinsic.json out_extrinsic.json [in_extrinsic.json]\n";
	std::cout << std::endl;
	std::exit(1);
}

int main(int argc, const char* argv[]) {
	if(argc <= 3) usage_fail();
	std::string in_correspondences_filename = argv[1];
	std::string in_intrinsic_filename = argv[2];
	std::string out_extrinsic_filename = argv[3];
	std::string in_extrinsic_filename;
	if(argc > 4) in_extrinsic_filename = argv[4];
	
	std::vector<calibration_correspondence> cors;
	decode_calibration_correspondences(import_json_file(in_correspondences_filename), std::inserter(cors, cors.end()));
	std::vector<cv::Vec3f> object_points;
	std::vector<cv::Vec2f> image_points;
	for(const calibration_correspondence& cor : cors) {
		object_points.push_back(cor.object_coordinates);
		image_points.push_back(cor.image_coordinates);
	}

	cv::Mat_<double> intrinsic = decode_mat_cv(import_json_file(in_intrinsic_filename));
	
	bool use_guess = false;
	cv::Vec3d rotation_vec, translation_vec;
	
	if(! in_extrinsic_filename.empty()) {
		std::cout << "using extrinsic guess" << std::endl;
		use_guess = true;
		cv::Mat_<double> rotation_mat(3, 3);
		cv::Mat_<double> extrinsic = decode_mat_cv(import_json_file(in_extrinsic_filename));
		for(int i = 0; i < 3; ++i) for(int j = 0; j < 3; ++j) rotation_mat(i, j) = extrinsic(i, j);
		for(int i = 0; i < 3; ++i) translation_vec[i] = extrinsic(i, 3);
		cv::Rodrigues(rotation_mat, rotation_vec);
	}
	
	cv::Mat distortion;
	cv::solvePnP(
		object_points,
		image_points,
		intrinsic,
		distortion,
		rotation_vec,
		translation_vec,
		use_guess
	);
	
	cv::Mat_<double> rotation_mat(3, 3);
	cv::Rodrigues(rotation_vec, rotation_mat);
	
	cv::Mat_<double> extrinsic(4, 4);
	for(int i = 0; i < 3; ++i) for(int j = 0; j < 3; ++j) extrinsic(i, j) = rotation_mat(i, j);
	for(int i = 0; i < 3; ++i) extrinsic(i, 3) = translation_vec[i];
	extrinsic(3, 0) = extrinsic(3, 1) = extrinsic(3, 2) = 0.0; extrinsic(3, 3) = 1.0;
	
	export_json_file(encode_mat(extrinsic), out_extrinsic_filename);
}

