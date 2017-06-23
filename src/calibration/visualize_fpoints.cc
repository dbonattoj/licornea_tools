#include <iostream>
#include <fstream>
#include <utility>
#include <random>
#include <vector>
#include <string>
#include "lib/feature_points.h"
#include "../lib/args.h"
#include "../lib/json.h"
#include "../lib/border.h"
#include "../lib/dataset.h"
#include "../lib/opencv.h"
#include "../lib/random_color.h"
#include "../lib/assert.h"

using namespace tlz;


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json fpoints.json out_visualization.png [dataset_group]");
	dataset datas = dataset_arg();
	feature_points fpoints = feature_points_arg();
	std::string visualization_filename = out_filename_arg();
	std::string dataset_group_name = string_opt_arg("");
	
	dataset_group datag = datas.group(dataset_group_name);
	
	std::cout << "loading background image" << std::endl;
	const cv::Vec3b background_color(0, 0, 0);
	cv::Mat_<cv::Vec3b> back_img(datag.image_size_with_border(), background_color);
	if(fpoints.view_idx) {
		view_index view_index = fpoints.view_idx;
		std::string image_filename = datag.view(view_index).image_filename();
		cv::Mat_<cv::Vec3b> img = cv::imread(image_filename, CV_LOAD_IMAGE_COLOR);
		cv::Mat_<uchar> gray_img;
		cv::cvtColor(img, gray_img, CV_BGR2GRAY);
		cv::cvtColor(gray_img, back_img, CV_GRAY2BGR);
	}
		
	std::cout << "drawing feature points" << std::endl;
	cv::Mat_<cv::Vec3b> img = visualize_feature_points(fpoints, back_img, datag.image_border());
	
	std::cout << "saving output visualization image" << std::endl;
	cv::imwrite(visualization_filename, img);
}

