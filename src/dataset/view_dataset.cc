#include "../lib/args.h"
#include "../lib/opencv.h"
#include "../lib/dataset.h"
#include "../lib/image_io.h"
#include "../lib/viewer.h"
#include "../lib/filesystem.h"
#include <string>

using namespace tlz;

const real max_viz_depth = 6000;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json [dataset_group]");
	dataset datas = dataset_arg();
	std::string dataset_group_name = string_opt_arg("");
	dataset_group datag = datas.group(dataset_group_name);
	
	cv::Size sz = datag.image_size_with_border();
	sz.height += 20;
	
	viewer view("Dataset Viewer", sz, true);
	auto& slider_x = view.add_int_slider("X", datas.x_mid(), datas.x_min(), datas.x_max(), datas.x_step());
	auto& slider_y = view.add_int_slider("Y", datas.y_mid(), datas.y_min(), datas.y_max(), datas.y_step());
	auto& depth_opacity_slider = view.add_real_slider("depth op", 0.0, 0.0, 1.0);
	auto& d_min_slider = view.add_real_slider("d min", 0, 0, max_viz_depth);
	auto& d_max_slider = view.add_real_slider("d max", max_viz_depth, 0, max_viz_depth);

	view.update_callback = [&]() {
		view_index idx(slider_x.value(), slider_y.value());
		if(! datas.valid(idx)) return;		
		
		std::string image_filename = datag.view(idx).image_filename();
		std::string depth_filename = datag.view(idx).depth_filename();

		view.clear();
		view.draw_text(cv::Rect(10, 0, sz.width-20, 20), "index: " + encode_view_index(idx));
		try {
			if(depth_opacity_slider == 1.0) {
				cv::Mat_<ushort> depth_img = load_depth(depth_filename);
				cv::Mat_<uchar> viz_depth_img = viewer::visualize_depth(depth_img, d_min_slider, d_max_slider);
				view.draw(cv::Point(0, 20), viz_depth_img);
			} else if(depth_opacity_slider == 0.0) {
				cv::Mat_<cv::Vec3b> img = load_texture(image_filename);
				view.draw(cv::Point(0, 20), img);
			} else {
				cv::Mat_<cv::Vec3b> img = load_texture(image_filename);
				cv::Mat_<ushort> depth_img = load_depth(depth_filename);
				cv::Mat_<uchar> viz_depth_img = viewer::visualize_depth(depth_img, d_min_slider, d_max_slider);
				cv::Mat_<cv::Vec3b> viz_depth_img_col;
				cv::cvtColor(viz_depth_img, viz_depth_img_col, CV_GRAY2BGR);
				cv::addWeighted(img, 1.0-depth_opacity_slider, viz_depth_img_col, depth_opacity_slider, 0.0, img);
				view.draw(cv::Point(0, 20), img);
			}
		} catch(const std::runtime_error&) {
			std::cout << "could not load " << image_filename << std::endl; 
		}
	};

	view.show_modal();
}
