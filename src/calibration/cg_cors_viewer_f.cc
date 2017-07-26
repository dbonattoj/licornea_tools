#include "../lib/args.h"
#include "../lib/opencv.h"
#include "../lib/intrinsics.h"
#include "../lib/dataset.h"
#include "../lib/misc.h"
#include "../lib/image_io.h"
#include "../lib/viewer.h"
#include "../lib/string.h"
#include "../lib/random_color.h"
#include "../lib/filesystem.h"
#include "../lib/image_io.h"
#include "lib/image_correspondence.h"
#include "lib/feature_points.h"
#include <cstdlib>
#include <iostream>
#include <string>

using namespace tlz;

const real max_viz_depth = 6000;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json cors.json closeup? [dataset_group]");
	dataset datas = dataset_arg();
	image_correspondences cors = image_correspondences_arg();
	bool closeup = bool_opt_arg("closeup");
	std::string dataset_group_name = string_opt_arg(cors.dataset_group);
	
	dataset_group datag = datas.group(dataset_group_name);
		
	cv::Size sz = datag.image_size_with_border();
	sz.height += 20;
	viewer view("Image Correspondences Viewer", sz, true);
	
	auto feature_names_s = get_feature_names(cors);
	std::vector<std::string> feature_names(feature_names_s.begin(), feature_names_s.end());
	
	auto& feature_slider = view.add_int_slider("feature", 0, 0, feature_names.size()-1);
	auto& x_slider = view.add_int_slider("X", datas.x_mid(), datas.x_min(), datas.x_max(), datas.x_step());
	auto& y_slider = view.add_int_slider("Y", datas.y_mid(), datas.y_min(), datas.y_max(), datas.y_step());
	auto& depth_opacity_slider = view.add_real_slider("depth op", 0.0, 0.0, 1.0);
	auto& d_min_slider = view.add_real_slider("d min", 0, 0, max_viz_depth);
	auto& d_max_slider = view.add_real_slider("d max", max_viz_depth, 0, max_viz_depth);
	auto& dots_opacity_slider = view.add_real_slider("dots op", 0.5, 0, 1.0);

	
	view.update_callback = [&]() {
		view_index idx(x_slider, y_slider);
		if(! datas.valid(idx)) return;

		const std::string& feature_name = feature_names.at(feature_slider);
		const image_correspondence_feature& feature = cors.features.at(feature_name);
		
		view.clear();
		
		cv::Mat_<cv::Vec3b> img;
		{
			std::string image_filename = datag.view(idx).image_filename();
			std::string depth_filename = datag.view(idx).depth_filename();

			cv::Mat_<uchar> gray_img = cv::imread(image_filename, CV_LOAD_IMAGE_GRAYSCALE);
			if(gray_img.empty()) {
				gray_img = cv::Mat_<uchar>(datag.image_size_with_border());
				gray_img.setTo(0);
			}
			
			if(depth_opacity_slider > 0.0 && file_exists(depth_filename)) {
				cv::Mat_<ushort> depth_img = load_depth(depth_filename);
				cv::Mat_<uchar> viz_depth_img = viewer::visualize_depth(depth_img, d_min_slider, d_max_slider);
				cv::addWeighted(gray_img, 1.0-depth_opacity_slider, viz_depth_img, depth_opacity_slider, 0.0, gray_img);
			}

			cv::cvtColor(gray_img, img, CV_GRAY2BGR);
		}

		cv::Vec3b col = random_color(string_hash(feature_name));

		try {
			if(closeup) {
				img = visualize_view_points_closeup(feature, img, col, idx, dots_opacity_slider, datag.image_border());
				view.draw(cv::Rect(0, 0, sz.width, sz.height), img);
			} else {
				img = visualize_view_points(feature, img, col, 1, datag.image_border());
				view.draw(cv::Point(0, 0), img);
			}
		} catch(const std::exception&) { return; }
		
		view.draw_text(cv::Rect(10, sz.height-20, sz.width-20, 20), "feature: " + feature_name + ", view: " + std::to_string(idx.x) + ", " + std::to_string(idx.y));
	};

	view.show_modal();
}
