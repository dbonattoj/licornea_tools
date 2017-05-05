#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <utility>
#include <random>
#include <vector>
#include <string>
#include "lib/image_correspondence.h"
#include "../lib/json.h"
#include "../lib/dataset.h"

using namespace tlz;

static cv::Vec3b random_color(int i) {
	static const int seed = 0;
	static std::vector<cv::Vec3b> colors;
	static std::mt19937 gen(seed);
	
	if(i < colors.size()) {
		return colors[i];
	} else {
		std::uniform_int_distribution<uchar> dist(0, 255);
		colors.push_back(cv::Vec3b(dist(gen), dist(gen), dist(gen)));
		return random_color(i);
	}
}

[[noreturn]] void usage_fail() {
	std::cout << "usage: visualize_image_correspondences dataset_parameters.json image_correspondences.json out_visualization.png\n";
	std::cout << std::endl;
	std::exit(1);
}


int main(int argc, const char* argv[]) {
	if(argc <= 3) usage_fail();
	std::string dataset_parameter_filename = argv[1];
	std::string cors_filename = argv[2];
	std::string visualization_filename = argv[3];

	std::cout << "loading data set" << std::endl;
	dataset datas(dataset_parameter_filename);
	
	std::cout << "loading image correspondences" << std::endl;
	image_correspondences cors = import_image_correspondences_file(cors_filename);
	
	std::cout << "loading reference image" << std::endl;
	view_index center_view_index = cors.reference;
	std::string center_image_filename = datas.view(center_view_index).image_filename();
	cv::Mat_<cv::Vec3b> center_img = cv::imread(center_image_filename, CV_LOAD_IMAGE_COLOR);
	cv::Mat_<uchar> center_gray_img;
	cv::cvtColor(center_img, center_gray_img, CV_BGR2GRAY);
	
	cv::Mat_<cv::Vec3b> out_img;
	cv::cvtColor(center_gray_img, out_img, CV_GRAY2BGR);

	
	std::cout << "drawing image correspondences" << std::endl;
	int i = 0;
	for(const auto& kv : cors.features) {
		const std::string& feature_name = kv.first;
		const image_correspondence_feature& feature = kv.second;
		
		cv::Vec3b col = random_color(i++);
		
		
		if(datas.is_1d()) {
			// draw circle		
			Eigen_vec2 center_point = feature.points.at(center_view_index);
			cv::Point center_point_cv(center_point[0], center_point[1]);
			cv::circle(out_img, center_point_cv, 10, cv::Scalar(col), 2);

			// draw label
			cv::putText(out_img, feature_name, center_point_cv, cv::FONT_HERSHEY_COMPLEX_SMALL, 1.0, cv::Scalar(col));
			
			// draw connecting line
			std::vector<cv::Point> trail_points;
			for(const auto& kv : feature.points) {
				Eigen_vec2 pt = kv.second;
				trail_points.emplace_back(pt[0], pt[1]);
			}

			std::vector<std::vector<cv::Point>> polylines = { trail_points };
			cv::polylines(out_img, polylines, false, cv::Scalar(col), 2);
			
		} else {
			// draw label		
			Eigen_vec2 center_point = feature.points.at(center_view_index);
			cv::Point center_point_cv(center_point[0] + 10, center_point[1] - 10);
			cv::putText(out_img, feature_name, center_point_cv, cv::FONT_HERSHEY_COMPLEX_SMALL, 1.0, cv::Scalar(col));

			// draw dot for each point
			for(const auto& kv : feature.points) {
				Eigen_vec2 pt = kv.second;
				
				cv::Point pt_cv(pt[0], pt[1]);
				cv::circle(out_img, pt_cv, 1, cv::Scalar(col), -1);
			}
		}
	}
	
	std::cout << "saving output visualization image" << std::endl;
	cv::imwrite(visualization_filename, out_img);
}

