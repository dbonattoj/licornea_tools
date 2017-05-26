#include <iostream>
#include <fstream>
#include <utility>
#include <random>
#include <vector>
#include <string>
#include "lib/image_correspondence.h"
#include "lib/cg/feature_slopes.h"
#include "../lib/json.h"
#include "../lib/dataset.h"
#include "../lib/opencv.h"
#include "../lib/random_color.h"

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: cg_visualize_optical_flow_slopes dataset_parameters.json intrinsics.json slopes.json out_visualization.png [width=200] [exaggeration=1]\n";
	std::cout << std::endl;
	std::exit(1);
}
int main(int argc, const char* argv[]) {
	if(argc <= 4) usage_fail();
	std::string dataset_parameter_filename = argv[1];
	std::string intrinsics_filename = argv[2];
	std::string slopes_filename = argv[3];
	std::string visualization_filename = argv[4];
	int width = 200;
	real exaggeration = 1.0;
	if(argc > 5) width = std::stoi(argv[5]);
	if(argc > 6) exaggeration = std::stof(argv[6]);

	std::cout << "loading data set" << std::endl;
	dataset datas(dataset_parameter_filename);
	
	std::cout << "loading intrinsics" << std::endl;
	intrinsics intr = decode_intrinsics(import_json_file(intrinsics_filename));

	std::cout << "loading slopes" << std::endl;
	feature_slopes fslopes = decode_feature_slopes(import_json_file(slopes_filename));
	
	std::cout << "loading background image" << std::endl;
	cv::Mat_<cv::Vec3b> back_img;
	{
		view_index view_index = fslopes.view_idx;
		std::string image_filename = datas.view(view_index).image_filename();
		cv::Mat_<cv::Vec3b> img = cv::imread(image_filename, CV_LOAD_IMAGE_COLOR);
		cv::Mat_<uchar> gray_img;
		cv::cvtColor(img, gray_img, CV_BGR2GRAY);
	
		cv::Mat_<uchar> undist_gray_img;
		cv::undistort(
			gray_img,
			undist_gray_img,
			intr.K,
			intr.distortion.cv_coeffs(),
			intr.K
		);
		
		cv::cvtColor(undist_gray_img, back_img, CV_GRAY2BGR);
	}
		
	std::cout << "drawing feature slopes" << std::endl;
	cv::Mat_<cv::Vec3b> img = visualize_feature_points(fslopes, back_img);
	img = visualize_feature_slopes(fslopes, img, width, exaggeration);
	
	std::cout << "saving output visualization image" << std::endl;
	cv::imwrite(visualization_filename, img);
}

