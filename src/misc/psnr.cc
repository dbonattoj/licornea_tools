#include "../lib/common.h"
#include "../lib/args.h"
#include "../lib/opencv.h"
#include <iostream>
#include <string>

using namespace tlz;


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "image1.png image2.png");
	std::string image1_filename = in_filename_arg();
	std::string image2_filename = in_filename_arg();
	
	cv::Mat mat1 = cv::imread(image1_filename, CV_LOAD_IMAGE_COLOR);
	cv::Mat mat2 = cv::imread(image2_filename, CV_LOAD_IMAGE_COLOR);
	
	cv::Mat squared_difference;
	cv::absdiff(mat1, mat2, squared_difference);
	squared_difference.convertTo(squared_difference, CV_32F);
	squared_difference = squared_difference.mul(squared_difference);
	
	cv::Scalar s = cv::sum(squared_difference);
	double sse = s.val[0] + s.val[1] + s.val[2];
	
	double psnr = 0.0;
	if(sse > 1e-10) {
		double mse = sse / (double)(mat1.channels() * mat1.total());
		psnr = 10.0 * log10((255 * 255) / mse);
	}
	
	std::cout << psnr << std::endl;
}

