#include "../lib/args.h"
#include "../lib/opencv.h"
#include "../lib/intrinsics.h"
#include "../lib/dataset.h"
#include "../lib/misc.h"
#include "../lib/image_io.h"
#include "../lib/viewer.h"
#include "../lib/string.h"
#include "../lib/random_color.h"
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
	
	auto feature_names_s = feature_names(cors);
	std::vector<std::string> feature_names(feature_names_s.begin(), feature_names_s.end());
	
	auto& feature_slider = view.add_int_slider("feature", 0, 0, feature_names.size()-1);

	
	view.update_callback = [&]() {
		const std::string& feature_name = feature_names.at(feature_slider);
		const image_correspondence_feature& feature = cors.features.at(feature_name);
		view_index idx = feature.reference_view;
		
		if(! datas.valid(idx)) return;
		
		cv::Mat_<cv::Vec3b> img;
		{
			std::string image_filename = datag.view(idx).image_filename();
			cv::Mat_<uchar> gray_img = cv::imread(image_filename, CV_LOAD_IMAGE_GRAYSCALE);
			if(gray_img.empty()) return;
			cv::cvtColor(gray_img, img, CV_GRAY2BGR);
		}

		cv::Vec3b col = random_color(string_hash(feature_name));
		img = visualize_view_points(feature, img, col, 2, datag.image_border());

		view.draw(cv::Point(0, 0), img);
	};

	view.show_modal();
}
