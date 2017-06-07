#include "../lib/args.h"
#include "../lib/opencv.h"
#include "../lib/intrinsics.h"
#include "../lib/dataset.h"
#include "../lib/misc.h"
#include "../lib/image_io.h"
#include "lib/image_correspondence.h"
#include "lib/feature_points.h"
#include <cstdlib>
#include <iostream>
#include <string>
#include <functional>

using namespace tlz;


std::function<void()> update_function;
void update_callback(int = 0, void* = nullptr) {	
	update_function();
}

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json cors.json [dataset_group]");
	dataset datas = dataset_arg();
	image_correspondences cors = image_correspondences_arg();
	std::string dataset_group = string_opt_arg(cors.dataset_group);
	
	border bord = decode_border(get_or(datas.group_parameters(dataset_group), "border", json::object()));
		
	std::cout << "running viewer" << std::endl;

	const std::string window_name = "Image Correspondences Viewer";

	int slider_x = datas.x_mid() - datas.x_min();
	int slider_y = datas.y_mid() - datas.y_min();
	
	auto update = [&]() {
		int x = slider_x + datas.x_min();
		int y = slider_y + datas.y_min();
		
		if(!datas.x_valid(x) || !datas.y_valid(y)) return;
		view_index idx(x, y);
		
		cv::Mat_<cv::Vec3b> back_img;
		{
			std::string image_filename = datas.view(idx).group_view(dataset_group).image_filename();
			cv::Mat_<uchar> img = cv::imread(image_filename, CV_LOAD_IMAGE_GRAYSCALE);
			if(img.empty()) return;
			cv::cvtColor(img, back_img, CV_GRAY2BGR);
		}
		
		cv::Mat_<cv::Vec3b> shown_img;

		if(datas.x_valid(x) && datas.y_valid(y)) {
			feature_points fpoints = feature_points_for_view(cors, idx);
			shown_img = visualize_feature_points(fpoints, back_img, bord);
		}
	
		cv::imshow(window_name, shown_img);
	};
	update_function = update;

	cv::namedWindow(window_name, CV_WINDOW_NORMAL);

	cv::createTrackbar("x", window_name, &slider_x, datas.x_max() - datas.x_min(), &update_callback);
	cv::createTrackbar("y", window_name, &slider_y, datas.y_max() - datas.y_min(), &update_callback);

	update();
	
	while(cv::waitKey(0) != escape_keycode);
}
