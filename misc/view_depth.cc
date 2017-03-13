#include "../lib/opencv.h"
#include <cstdlib>
#include <iostream>
#include <string>

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: view_depth depth.png [z_near z_far]" << std::endl;
	std::exit(EXIT_FAILURE);
}

bool is16bit = false;
int slider_begin = -1;
int slider_step = -1;
int slider_range = -1;
int z_near_slider_value = -1;
int z_far_slider_value = -1;
cv::Mat_<ushort> depth;
cv::Mat_<uchar> shown_depth;
std::string window_name;
 
void update_depth(int = 0, void* = nullptr) {
	ushort z_near = slider_begin + (z_near_slider_value * slider_step);
	ushort z_far = slider_begin + (z_far_slider_value * slider_step);
	std::cout << "z_near=" << z_near << "    z_far=" << z_far << std::endl;
	
	double alpha = 255.0 / (z_far - z_near);
	double beta = -alpha * z_near;

	cv::convertScaleAbs(depth, shown_depth, alpha, beta);
	shown_depth.setTo(0, (depth == 0));
	
	cv::imshow(window_name, shown_depth);
}


int main(int argc, const char* argv[]) {
	if(argc <= 1) usage_fail();
	std::string depth_filename = argv[1];
	int z_near = -1, z_far = -1;
	if(argc > 2) z_near = std::atoi(argv[2]);
	if(argc > 3) z_far = std::atoi(argv[3]);
	
	depth = cv::imread(depth_filename, CV_LOAD_IMAGE_ANYDEPTH);
	is16bit = (depth.depth() == CV_16U);

	// scale if too large
	int max_cols = 1000;
	if(depth.cols > max_cols) {
		int new_cols = max_cols;
		int new_rows = new_cols * depth.rows / depth.cols;
		cv::resize(depth, depth, cv::Size(new_cols, new_rows));
	}

	// get min/max value in image (excluding 0 pixels)
	ushort min_value = (is16bit ? 0xffff : 0xff), max_value = 0;
	for(ushort value : depth) {
		if(value == 0) continue;
		else if(value > max_value) max_value = value;
		else if(value < min_value) min_value = value;
	}
	slider_begin = min_value;
	slider_range = max_value - min_value;
	
	int max_range = 200;
	if(slider_range > max_range) slider_step = slider_range / max_range;
	else slider_step = 1;
		
	// initialize slider values
	if(z_near != -1) z_near_slider_value = (z_near - slider_begin)/slider_step;
	else z_near_slider_value = 0;

	if(z_far != -1) z_far_slider_value = (z_far - slider_begin)/slider_step;
	else z_far_slider_value = 0;

	// setup window
	window_name = depth_filename;
	
	cv::namedWindow(window_name, CV_WINDOW_AUTOSIZE);
	cv::createTrackbar("1. z_near", window_name, &z_near_slider_value, slider_range / slider_step, &update_depth);
	cv::createTrackbar("2. z_far", window_name, &z_far_slider_value, slider_range / slider_step, &update_depth);

	// initialize depth and show
	update_depth();
	
	// wait for user
	cv::waitKey(0);
}
