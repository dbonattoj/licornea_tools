#include "../lib/args.h"
#include "../lib/opencv.h"
#include "../lib/intrinsics.h"
#include "../lib/dataset.h"
#include "../lib/misc.h"
#include "../lib/image_io.h"
#include "../lib/viewer.h"
#include "lib/image_correspondence.h"
#include "lib/feature_points.h"
#include <cstdlib>
#include <iostream>
#include <string>

using namespace tlz;


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json cors.json [dataset_group]");
	dataset datas = dataset_arg();
	image_correspondences cors = image_correspondences_arg();
	std::string dataset_group_name = string_opt_arg(cors.dataset_group);
	
	dataset_group datag = datas.group(dataset_group_name);
		
	viewer view("Image Correspondences Viewer", datag.image_size_with_border(), true);
	auto& x_slider = view.add_int_slider("X", datas.x_mid(), datas.x_min(), datas.x_max(), datas.x_step());
	auto& y_slider = view.add_int_slider("Y", datas.y_mid(), datas.y_min(), datas.y_max(), datas.y_step());

	
	view.update_callback = [&]() {
		view_index idx(x_slider, y_slider);
		if(! datas.valid(idx)) return;
		
		cv::Mat_<cv::Vec3b> img;
		{
			std::string image_filename = datag.view(idx).image_filename();
			cv::Mat_<uchar> gray_img = cv::imread(image_filename, CV_LOAD_IMAGE_GRAYSCALE);
			if(gray_img.empty()) {
				gray_img = cv::Mat_<uchar>(datag.image_size_with_border());
				gray_img.setTo(0);
			}
			cv::cvtColor(gray_img, img, CV_GRAY2BGR);
		}
		feature_points fpoints = feature_points_for_view(cors, idx);
		img = visualize_feature_points(fpoints, img, datag.image_border());

		view.draw(cv::Point(0, 0), img);
	};

	view.show_modal();
}
