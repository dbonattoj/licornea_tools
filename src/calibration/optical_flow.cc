#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <utility>
#include <random>
#include <vector>
#include <string>
#include "../lib/json.h"
#include "../lib/dataset.h"
#include "../lib/image_correspondence.h"

using namespace tlz;


static std::string feature_name(int i) {
	return "pt" + std::to_string(i);
}


[[noreturn]] void usage_fail() {
	std::cout << "usage: optical_flow dataset_parameters.json out_image_correspondences.json\n";
	std::cout << std::endl;
	std::exit(1);
}



int main(int argc, const char* argv[]) {
	if(argc <= 2) usage_fail();
	std::string dataset_parameter_filename = argv[1];
	std::string out_cors_filename = argv[2];

	std::cout << "loading data set" << std::endl;
	dataset set(dataset_parameter_filename);
	if(set.is_2d()) {
		std::cout << "no support for 2d dataset" << std::endl;
		std::exit(1);
	}
	
	int mid_x = ((set.x_max() + set.x_min()) / (2*set.x_step())) * set.x_step();
	std::cout << mid_x << std::endl;
		
	std::cout << "loading center image" << std::endl;
	std::cout << set.view(mid_x).image_filename() << std::endl;
	
	cv::Mat_<cv::Vec3b> center_col_img = cv::imread(set.view(mid_x).image_filename(), CV_LOAD_IMAGE_COLOR);
	cv::Mat_<uchar> center_gray_img;
	cv::cvtColor(center_col_img, center_gray_img, CV_BGR2GRAY);
		
	std::cout << "finding good points" << std::endl;
	const std::size_t points_count = 10;
	std::vector<cv::Point2f> center_points(points_count);
	cv::goodFeaturesToTrack(center_gray_img, center_points, points_count, 0.3, 7);
		
	cv::Mat_<uchar> img;
	std::vector<cv::Point2f> points;
	std::vector<uchar> status;
	std::vector<image_correspondence_feature> correspondences(points_count);
	std::vector<std::vector<cv::Point2f>> views_points;

	auto flow_to = [&img, &points, &status, &set](int x) {
		std::vector<cv::Point2f> new_points(points_count);
		cv::Mat_<cv::Vec3b> new_col_img = cv::imread(set.view(x).image_filename(), CV_LOAD_IMAGE_COLOR);
		cv::Mat_<uchar> new_img;
		cv::cvtColor(new_col_img, new_img, CV_BGR2GRAY);

		std::vector<uchar> new_status(points_count);
		std::vector<float> err(points_count);
		cv::calcOpticalFlowPyrLK(img, new_img, points, new_points, new_status, err);
		for(std::ptrdiff_t i = 0; i < points_count; ++i)
			if(!status[i] || !new_status[i]) new_points[i] = cv::Point2f(0.0, 0.0);

		img = new_img;
		points = new_points;
		status = new_status;
	};
		
	auto add_correspondences = [&correspondences, &points, points_count](int x) {
		image_correspondence_feature::view_index_type idx(x, -1);
		for(std::ptrdiff_t pt = 0; pt < points_count; ++pt)
			if(points[pt].x != 0 && points[pt].y != 0)
				correspondences[pt].points[idx] = Eigen_vec2(points[pt].x, points[pt].y);
	};
	
	std::cout << "optical flow by increasing x starting at mid_x..." << std::endl;

	img = center_gray_img;
	points = center_points;
	status.assign(points_count, 1);
	
	add_correspondences(mid_x);
	
	for(int x = mid_x + set.x_step(); x <= set.x_max(); x += set.x_step()) {
		std::cout << "   x=" << (x - set.x_step()) << " --> x=" << x << std::endl;
		flow_to(x);
		add_correspondences(x);
		views_points.push_back(points);
	}
	
	
	std::cout << "optical flow by decreasing x starting at mid_x..." << std::endl;
	std::vector<std::vector<cv::Point2f>> x_decr_frames;

	img = center_gray_img;
	points = center_points;
	status.assign(points_count, 1);
	for(int x = mid_x - set.x_step(); x >= set.x_min(); x -= set.x_step()) {
		std::cout << "   x=" << (x + set.x_step()) << " <-- x=" << x << std::endl;
		flow_to(x);
		add_correspondences(x);
		views_points.insert(views_points.begin(), points);
	}
	

	std::cout << "saving image correspondences" << std::endl;
	json j_cors = json::object();
	for(std::ptrdiff_t pt = 0; pt < points_count; ++pt) {
		std::string name = feature_name(pt);
		j_cors[name] = encode_image_correspondence_feature(correspondences[pt]);
	}
	export_json_file(j_cors, out_cors_filename);
}

