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
	get_args(argc, argv, "dataset_parameters.json image_correspondences.json out_visualization.png [feature_name]");
	dataset datas = dataset_arg();
	image_correspondences cors = image_correspondences_arg();
	std::string visualization_filename = out_filename_arg();
	std::string dataset_group = cors.dataset_group;
	std::string feature_to_viz = string_opt_arg("");
	
	border bord = decode_border(get_or(datas.group_parameters(dataset_group), "border", json::object()));

	std::cout << "loading reference image" << std::endl;
	view_index center_view_index = cors.reference;
	std::string center_image_filename = datas.view(center_view_index).group_view(dataset_group).image_filename();
	cv::Mat_<cv::Vec3b> center_img = cv::imread(center_image_filename, CV_LOAD_IMAGE_COLOR);
	cv::Mat_<uchar> center_gray_img;
	cv::cvtColor(center_img, center_gray_img, CV_BGR2GRAY);
	
	cv::Mat_<cv::Vec3b> img;
	cv::cvtColor(center_gray_img, img, CV_GRAY2BGR);
		
	std::cout << "drawing image correspondences" << std::endl;

	for(const auto& kv : cors.features) {
		const std::string& feature_name = kv.first;
		const image_correspondence_feature& feature = kv.second;
		if(! feature_to_viz.empty() && feature_name != feature_to_viz) continue;
		
		cv::Vec3b col = random_color(string_hash(feature_name));
		
		if(datas.is_1d()) {
			// draw circle		
			vec2 center_point = feature.points.at(center_view_index);
			cv::Point center_point_cv(bord.left + center_point[0], bord.top + center_point[1]);
			cv::circle(img, center_point_cv, 10, cv::Scalar(col), 2);

			// draw label
			cv::putText(img, feature_name, center_point_cv, cv::FONT_HERSHEY_COMPLEX_SMALL, 1.0, cv::Scalar(col));
			
			// draw connecting line
			std::vector<cv::Point> trail_points;
			for(const auto& kv : feature.points) {
				vec2 pt = kv.second;
				pt[0] += bord.left; pt[1] += bord.top;
				trail_points.emplace_back(pt[0], pt[1]);
			}

			std::vector<std::vector<cv::Point>> polylines = { trail_points };
			cv::polylines(img, polylines, false, cv::Scalar(col), 2);
			
		} else {
			// draw label		
			vec2 center_point = feature.points.at(center_view_index);
			cv::Point center_point_cv(bord.left + center_point[0] + 20, bord.top + center_point[1] - 20);
			cv::putText(img, feature_name, center_point_cv, cv::FONT_HERSHEY_COMPLEX_SMALL, 1.0, cv::Scalar(col));

			// draw dot for each point
			for(const auto& kv : feature.points) {
				vec2 pt = kv.second;
				pt[0] += bord.left; pt[1] += bord.top;
				
				cv::Point pt_cv(pt[0], pt[1]);
				//if(cv::Rect(cv::Point(), img.size()).contains(pt_cv)) img(pt_cv) = col;
				cv::circle(img, pt_cv, 2, cv::Scalar(col), -1);
			}
		}
	}
	
	std::cout << "saving output visualization image" << std::endl;
	cv::imwrite(visualization_filename, img);
}

