#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <utility>
#include <random>
#include <vector>
#include "../lib/json.h"
#include "../lib/dataset.h"
#include "../lib/image_correspondence.h"

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: optical_flow parameter.json out_cors.json\n";
	std::cout << std::endl;
	std::exit(1);
}

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


static void export_points_visualization(
	const cv::Mat_<uchar>& img,
	const std::vector<cv::Point2f>& points,
	const std::string& filename
) {
	cv::Mat_<cv::Vec3b> out_img;
	cv::cvtColor(img, out_img, CV_GRAY2BGR);
	int i = 0;
	for(cv::Point pt : points) {
		cv::Vec3b col = random_color(i++);
		cv::circle(out_img, pt, 10, cv::Scalar(col), 2);
	}
	cv::imwrite(filename, out_img);
}


static void export_trails_visualization(
	const cv::Mat_<uchar>& img,
	const std::vector<std::vector<cv::Point2f>>& frames,
	const std::string& filename
) {
	cv::Mat_<cv::Vec3b> out_img;
	cv::cvtColor(img, out_img, CV_GRAY2BGR);
	const std::vector<cv::Point2f>& initial_points = frames.front();
	for(std::ptrdiff_t i = 0; i < initial_points.size(); ++i) {
		cv::Vec3b col = random_color(i);
		std::vector<cv::Point> trail_points;
		for(const std::vector<cv::Point2f>& frame_points : frames) {
			cv::Point2f pt = frame_points[i];
			if(pt.x == 0 && pt.y == 0) break;
			else trail_points.push_back(pt);
		}
		
		std::vector<std::vector<cv::Point>> polylines = { trail_points };
		cv::polylines(out_img, polylines, false, cv::Scalar(col), 2);
	}
	
	cv::imwrite(filename, out_img);
}


int main(int argc, const char* argv[]) {
	if(argc <= 2) usage_fail();
	std::string dataset_parameter_filename = argv[1];
	std::string out_cors_filename = argv[2];

	dataset set(dataset_parameter_filename);

	if(set.is_2d()) {
		std::cout << "no support for 2d dataset" << std::endl;
		std::exit(1);
	}
	
	int mid_x = ((set.x_max() + set.x_min()) / (2*set.x_step())) * set.x_step();
	std::cout << mid_x << std::endl;
		
	std::cout << "loading center image" << std::endl;
	std::cout << set.view(mid_x).raw_texture_filename() << std::endl;
	
	cv::Mat_<cv::Vec3b> center_col_img = cv::imread(set.view(mid_x).raw_texture_filename(), CV_LOAD_IMAGE_COLOR);
	cv::Mat_<uchar> center_gray_img;
	cv::cvtColor(center_col_img, center_gray_img, CV_BGR2GRAY);
		
	std::cout << "finding good points" << std::endl;
	const std::size_t points_count = 200;
	std::vector<cv::Point2f> center_points(points_count);
	cv::goodFeaturesToTrack(center_gray_img, center_points, points_count, 0.3, 7);
	
	//export_points_visualization(center_gray_img, center_points, "viz/center_points.png");
	
	cv::Mat_<uchar> img;
	std::vector<cv::Point2f> points;
	std::vector<uchar> status;
	std::vector<image_correspondence_feature> correspondences(points_count);

	auto flow_to = [&img, &points, &status, &set](int x) {
		std::vector<cv::Point2f> new_points(points_count);
		cv::Mat_<cv::Vec3b> new_col_img = cv::imread(set.view(x).raw_texture_filename(), CV_LOAD_IMAGE_COLOR);
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
	std::vector<std::vector<cv::Point2f>> x_incr_frames;

	img = center_gray_img;
	points = center_points;
	status.assign(points_count, 1);
	for(int x = mid_x + set.x_step(); x <= set.x_max(); x += set.x_step()) {
		std::cout << "   x=" << (x - set.x_step()) << " --> x=" << x << std::endl;
		flow_to(x);
		add_correspondences(x);
		x_incr_frames.push_back(points);
	}
	export_trails_visualization(center_gray_img, x_incr_frames, "viz/trails+.png");
	
	
	std::cout << "optical flow by decreasing x starting at mid_x..." << std::endl;
	std::vector<std::vector<cv::Point2f>> x_decr_frames;

	img = center_gray_img;
	points = center_points;
	status.assign(points_count, 1);
	for(int x = mid_x - set.x_step(); x >= set.x_min(); x -= set.x_step()) {
		std::cout << "   x=" << (x + set.x_step()) << " <-- x=" << x << std::endl;
		flow_to(x);
		add_correspondences(x);
		x_decr_frames.push_back(points);
	}
	export_trails_visualization(center_gray_img, x_decr_frames, "viz/trails-.png");


	std::cout << "saving image correspondences" << std::endl;
	json j_cors = json::object();
	for(std::ptrdiff_t pt = 0; pt < points_count; ++pt) {
		std::string point_name = "pt" + std::to_string(pt);
		j_cors[point_name] = encode_image_correspondence_feature(correspondences[pt]);
	}
	export_json_file(j_cors, out_cors_filename);
}

