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
#include "../lib/border.h"
#include "../lib/dataset.h"
#include "../lib/opencv.h"
#include "../lib/random_color.h"
#include "../lib/assert.h"

using namespace tlz;


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json slopes.json out_visualization.png [width=200] [exaggeration=1] [dataset_group]");
	dataset datas = dataset_arg();
	feature_slopes fslopes = feature_slopes_arg();
	std::string visualization_filename = out_filename_arg();
	int seg_width = int_opt_arg(200);
	real exaggeration = real_opt_arg(1.0);	
	std::string dataset_group_name = string_opt_arg("");
	
	dataset_group datag = datas.group(dataset_group_name);
	
	std::cout << "loading background image" << std::endl;
	const cv::Vec3b background_color(0, 0, 0);
	cv::Mat_<cv::Vec3b> back_img(datag.image_size_with_border(), background_color);
	if(fslopes.view_idx) {
		view_index view_index = fslopes.view_idx;
		std::string image_filename = datag.view(view_index).image_filename();
		cv::Mat_<cv::Vec3b> img = cv::imread(image_filename, CV_LOAD_IMAGE_COLOR);
		cv::Mat_<uchar> gray_img;
		cv::cvtColor(img, gray_img, CV_BGR2GRAY);
		cv::cvtColor(gray_img, back_img, CV_GRAY2BGR);
	}
		
	std::cout << "drawing feature slopes" << std::endl;
	cv::Mat_<cv::Vec3b> img = visualize_feature_points(fslopes, back_img, datag.image_border());
	img = visualize_feature_slopes(fslopes, img, seg_width, exaggeration, 2, datag.image_border());
	
	std::cout << "saving output visualization image" << std::endl;
	cv::imwrite(visualization_filename, img);
}

