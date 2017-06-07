#include <iostream>
#include <fstream>
#include <utility>
#include <random>
#include <vector>
#include <string>
#include "lib/image_correspondence.h"
#include "../lib/args.h"
#include "../lib/json.h"
#include "../lib/random_color.h"
#include "../lib/dataset.h"
#include "../lib/border.h"
#include "../lib/opencv.h"
#include "../lib/string.h"

using namespace tlz;

const cv::Vec3b black(0, 0, 0);
const cv::Vec3b white(255, 255, 255);

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json image_correspondences.json out_visualization.png feature_name [dot_radius=2]");
	dataset datas = dataset_arg();
	image_correspondences cors = image_correspondences_arg();
	std::string visualization_filename = out_filename_arg();
	std::string dataset_group = cors.dataset_group;
	std::string feature_name = string_arg();
	int dot_radius = int_opt_arg(2);
	
	border bord = decode_border(get_or(datas.group_parameters(dataset_group), "border", json::object()));

	const image_correspondence_feature& feature = cors.features.at(feature_name);
	view_index reference_idx = feature.reference_view;

	std::cout << "loading reference image" << std::endl;
	cv::Mat_<cv::Vec3b> img;
	{
		std::string reference_image_filename = datas.view(reference_idx).group_view(dataset_group).image_filename();
		cv::Mat_<cv::Vec3b> reference_image = cv::imread(reference_image_filename, CV_LOAD_IMAGE_COLOR);
		cv::Mat_<uchar> reference_gray_img;
		cv::cvtColor(reference_image, reference_gray_img, CV_BGR2GRAY);
		cv::cvtColor(reference_gray_img, img, CV_GRAY2BGR);
	}
		
	std::cout << "drawing image correspondences" << std::endl;	
	cv::Vec3b col = random_color(string_hash(feature_name));
	
	if(datas.is_1d()) {
		// draw circle		
		vec2 center_point = feature.points.at(reference_idx).position;
		cv::Point center_point_cv(bord.left + center_point[0], bord.top + center_point[1]);
		cv::circle(img, center_point_cv, 10, cv::Scalar(col), 2);

		// draw label
		cv::putText(img, feature_name, center_point_cv, cv::FONT_HERSHEY_COMPLEX_SMALL, 1.0, cv::Scalar(col));
		
		// draw connecting line
		std::vector<cv::Point> trail_points;
		for(const auto& kv : feature.points) {
			vec2 pt = kv.second.position;
			pt[0] += bord.left; pt[1] += bord.top;
			trail_points.emplace_back(pt[0], pt[1]);
		}

		std::vector<std::vector<cv::Point>> polylines = { trail_points };
		cv::polylines(img, polylines, false, cv::Scalar(col), 2);
		
	} else {
		// draw label		
		vec2 center_point = feature.points.at(reference_idx).position;
		cv::Point center_point_cv(bord.left + center_point[0] + 20, bord.top + center_point[1] - 20);
		cv::putText(img, feature_name, center_point_cv, cv::FONT_HERSHEY_COMPLEX_SMALL, 1.0, cv::Scalar(col));

		// draw dot for each point
		for(const auto& kv : feature.points) {
			vec2 pt = kv.second.position;
			pt[0] += bord.left; pt[1] += bord.top;
			
			cv::Point pt_cv(pt[0], pt[1]);
			if(dot_radius <= 1) {
				if(cv::Rect(cv::Point(), img.size()).contains(pt_cv)) img(pt_cv) = col;
			} else {
				cv::circle(img, pt_cv, 2, cv::Scalar(col), -1);
			}
		}
	}
	
	std::cout << "saving output visualization image" << std::endl;
	cv::imwrite(visualization_filename, img);
}

