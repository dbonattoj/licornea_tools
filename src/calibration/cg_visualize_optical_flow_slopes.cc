#include <iostream>
#include <fstream>
#include <utility>
#include <random>
#include <vector>
#include <string>
#include "lib/image_correspondence.h"
#include "lib/cg/feature_slopes.h"
#include "../lib/args.h"
#include "../lib/json.h"
#include "../lib/dataset.h"
#include "../lib/opencv.h"
#include "../lib/random_color.h"

using namespace tlz;


int main(int argc, const char* argv[]) {
	get_args(argc, argv,
		"dataset_parameters.json intrinsics.json slopes.json out_visualization.png [width=200] [exaggeration=1]");
	dataset datas = dataset_arg();
	intrinsics intr = intrinsics_arg();
	feature_slopes fslopes = feature_slopes_arg();
	std::string visualization_filename = out_filename_arg();
	int width = int_opt_arg(200);
	real exaggeration = real_opt_arg(1.0);
	
	std::cout << "loading background image" << std::endl;
	cv::Mat_<cv::Vec3b> back_img;
	{
		view_index view_index = fslopes.view_idx;
		std::string image_filename = datas.view(view_index).image_filename();
		cv::Mat_<cv::Vec3b> img = cv::imread(image_filename, CV_LOAD_IMAGE_COLOR);
		cv::Mat_<uchar> gray_img;
		cv::cvtColor(img, gray_img, CV_BGR2GRAY);
	
		cv::Mat_<uchar> undist_gray_img;
		if(intr.distortion)
			cv::undistort(
				gray_img,
				undist_gray_img,
				intr.K,
				intr.distortion.cv_coeffs(),
				intr.K
			);
		else
			undist_gray_img = gray_img;
		
		cv::cvtColor(undist_gray_img, back_img, CV_GRAY2BGR);
	}
		
	std::cout << "drawing feature slopes" << std::endl;
	cv::Mat_<cv::Vec3b> img = visualize_feature_points(fslopes, back_img);
	img = visualize_feature_slopes(fslopes, img, width, exaggeration);
	
	std::cout << "saving output visualization image" << std::endl;
	cv::imwrite(visualization_filename, img);
}

