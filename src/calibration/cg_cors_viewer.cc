#include "../lib/args.h"
#include "../lib/opencv.h"
#include "../lib/intrinsics.h"
#include "../lib/dataset.h"
#include "../lib/misc.h"
#include "lib/image_correspondence.h"
#include "lib/cg/feature_points.h"
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
	get_args(argc, argv, 
		"dataset_parameters.json cors.json");
	dataset datas = dataset_arg();
	image_correspondences cors = image_correspondences_arg();
	view_index reference_idx = cors.reference;
	std::string dataset_group = cors.dataset_group;
	border bord = decode_border(get_or(datas.group_parameters(dataset_group), "border", json::object()));
	
	feature_points ref_fpoints = feature_points_for_view(cors, reference_idx);
	
	
	std::cout << "running viewer" << std::endl;

	const std::string window_name = "Image Correspondences Viewer";

	int x = reference_idx.x;
	int y = reference_idx.y;
	
	auto update = [&]() {
		if(!datas.x_valid(x) || !datas.y_valid(y)) return;
		view_index idx(x, y);
		
		cv::Mat_<cv::Vec3b> back_img;
		{
			std::string image_filename = datas.view(idx).group_view(dataset_group).image_filename();
			cv::Mat_<uchar> img = cv::imread(image_filename, CV_LOAD_IMAGE_GRAYSCALE);
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

	cv::createTrackbar("x", window_name, &x, datas.x_max(), &update_callback);
	cv::createTrackbar("y", window_name, &y, datas.y_max(), &update_callback);

	update();
	
	while(cv::waitKey(0) != escape_keycode);
}
