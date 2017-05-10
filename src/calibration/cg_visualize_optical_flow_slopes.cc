#include <iostream>
#include <fstream>
#include <utility>
#include <random>
#include <vector>
#include <string>
#include "lib/image_correspondence.h"
#include "../lib/json.h"
#include "../lib/dataset.h"
#include "../lib/opencv.h"
#include "../lib/random_color.h"

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: cg_visualize_optical_flow_slopes dataset_parameters.json slopes.json out_visualization.png [width=200] [exaggeration=1]\n";
	std::cout << std::endl;
	std::exit(1);
}
int main(int argc, const char* argv[]) {
	if(argc <= 3) usage_fail();
	std::string dataset_parameter_filename = argv[1];
	std::string slopes_filename = argv[2];
	std::string visualization_filename = argv[3];
	int width = 200;
	real exaggeration = 1.0;
	if(argc > 4) width = std::stoi(argv[4]);
	if(argc > 5) exaggeration = std::stof(argv[5]);

	std::cout << "loading data set" << std::endl;
	dataset datas(dataset_parameter_filename);
	
	std::cout << "loading slopes" << std::endl;
	json j_slopes = import_json_file(slopes_filename);
	json j_feature_slopes = j_slopes["slopes"];
	
	std::cout << "loading reference image" << std::endl;
	view_index center_view_index = decode_view_index(j_slopes["reference"]);
	std::string center_image_filename = datas.view(center_view_index).image_filename();
	cv::Mat_<cv::Vec3b> center_img = cv::imread(center_image_filename, CV_LOAD_IMAGE_COLOR);
	cv::Mat_<uchar> center_gray_img;
	cv::cvtColor(center_img, center_gray_img, CV_BGR2GRAY);
	
	cv::Mat_<cv::Vec3b> out_img;
	cv::cvtColor(center_gray_img, out_img, CV_GRAY2BGR);
	
	std::cout << "drawing feature slopes" << std::endl;
	real radius = width / 2.0;
	int i = 0;
	for(auto it = j_feature_slopes.begin(); it != j_feature_slopes.end(); ++it) {
		const std::string& feature_name = it.key();
		const json& j_slope = it.value();
		
		cv::Vec3b col = random_color(i++);
		
		cv::Point center_point(j_slope["ix"], j_slope["iy"]);
		real horizontal_slope = j_slope["horizontal"];
		real vertical_slope = j_slope["vertical"];
		
		horizontal_slope *= exaggeration;
		vertical_slope *= exaggeration;
		
		// draw circle		
		cv::circle(out_img, center_point, 10, cv::Scalar(col), 2);

		// draw label
		cv::Point label_point(center_point.x + 10, center_point.y - 10);
		cv::putText(out_img, feature_name, label_point, cv::FONT_HERSHEY_COMPLEX_SMALL, 1.0, cv::Scalar(col));
			
		// draw horizontal line segment
		{
			std::vector<cv::Point> end_points(2);
			end_points[0] = cv::Point(center_point.x - radius, center_point.y - radius*horizontal_slope);
			end_points[1] = cv::Point(center_point.x + radius, center_point.y + radius*horizontal_slope);
			std::vector<std::vector<cv::Point>> polylines { end_points };
			cv::polylines(out_img, polylines, false, cv::Scalar(col), 2);
		}


		// draw vertical line
		{
			std::vector<cv::Point> end_points(2);
			end_points[0] = cv::Point(center_point.x - radius*vertical_slope, center_point.y - radius);
			end_points[1] = cv::Point(center_point.x + radius*vertical_slope, center_point.y + radius);
			std::vector<std::vector<cv::Point>> polylines { end_points };
			cv::polylines(out_img, polylines, false, cv::Scalar(col), 2);
		}
	}
	
	std::cout << "saving output visualization image" << std::endl;
	cv::imwrite(visualization_filename, out_img);
}

