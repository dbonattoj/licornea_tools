#include <opencv2/opencv.hpp>
#include <iostream>
#include <random>
#include "../lib/json.h"
#include "../lib/dataset.h"

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: optical_flow parameter.json out_flow.json\n";
	std::cout << std::endl;
	std::exit(1);
}

static cv::Vec3b random_color(int i) {
	static std::vector<cv::Vec3b> colors;
	static std::mt19937 gen;
	
	if(i < colors.size()) {
		return colors[i];
	} else {
		std::uniform_int_distribution<uchar> dist(0, 255);
		colors.push_back(cv::Vec3b(dist(gen), dist(gen), dist(gen)));
		return random_color(i);
	}
}

int main(int argc, const char* argv[]) {
	if(argc <= 2) usage_fail();
	std::string dataset_parameter_filename = argv[1];
	std::string out_flow_filename = argv[2];

	dataset set(dataset_parameter_filename);

	if(set.is_2d()) {
		std::cout << "no support for 2d dataset" << std::endl;
		std::exit(1);
	}
	
	auto x_indices = set.x_indices();
	int mid_x = x_indices[x_indices.size()/2];
		
	std::cout << "loading center image" << std::endl;
	std::cout << set.view(mid_x).raw_texture_filename() << std::endl;
	
	cv::Mat_<cv::Vec3b> center_col_img = cv::imread(set.view(mid_x).raw_texture_filename(), CV_LOAD_IMAGE_COLOR);
	cv::Mat_<uchar> center_gray_img;
	cv::cvtColor(center_col_img, center_gray_img, CV_BGR2GRAY);
		
	std::cout << "finding good features" << std::endl;
	std::vector<cv::Point> corners(10);
	cv::goodFeaturesToTrack(center_gray_img, corners, 10, 0.3, 7);
	
	cv::Mat_<cv::Vec3b> out_img;
	cv::cvtColor(center_gray_img, out_img, CV_GRAY2BGR);
	int i = 0;
	for(cv::Point corner : corners) {
		cv::Vec3b col = random_color(i++);
		cv::circle(out_img, corner, 10, cv::Scalar(col), 2);
	}
	cv::imwrite("features.png", center_col_img);
}

