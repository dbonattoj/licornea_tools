#include "../lib/common.h"
#include "../lib/args.h"
#include "../lib/viewer.h"
#include "../lib/opencv.h"
#include <cstdlib>
#include <iostream>
#include <string>

using namespace tlz;


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "depth.png [z_near z_far] [out.png]");
	std::string depth_filename = in_filename_arg();
	int z_near_init = int_opt_arg(-1);
	int z_far_init = int_opt_arg(-1);
	std::string out_depth_filename = out_filename_opt_arg();
		
	depth = cv::imread(depth_filename, CV_LOAD_IMAGE_ANYDEPTH);
	is16bit = (depth.depth() == CV_16U);

	// scale if too large (not when saving to file)
	int max_cols = 1000;
	if(out_depth_filename.empty() && depth.cols > max_cols) {
		int new_cols = max_cols;
		int new_rows = new_cols * depth.rows / depth.cols;
		cv::resize(depth, depth, cv::Size(new_cols, new_rows));
	}

	// get min/max value in image (excluding 0 pixels)
	ushort min_value = (is16bit ? 0xffff : 0xff), max_value = 0;
	for(ushort value : depth) {
		if(value == 0) continue;
		else if(value > max_value) max_value = value;
		else if(value < min_value) min_value = value;
	}
	
	const int slider_steps = 200;
	
	if(out_depth_filename.empty()) {
		viewer view("Depth map");
		auto& z_near_slider = view.add_real_slider("z near", (z_near_init == -1 ? min_value : z_near_init), min_value, max_value, slider_steps);
		auto& z_far_slider = view.add_real_slider("z far", (z_far_init == -1 ? max_value : z_near_init), min_value, max_value, slider_steps);
		
		auto update = [&]() {
			real min_d = z_near_slider.value();
			real max_d = z_far_slider.value();
			
			cv::Mat_<uchar> img = viewer::visualize_depth(depth, min_d, max_d);
			view.clear(img.size());
			view.draw(img);
		};
		view.update_callback = update;
		view.show_modal();

	} else {
		cv::Mat_<uchar> img = viewer::visualize_depth(depth, z_near_init, z_far_init);
		cv::imwrite(out_depth_filename, img);
		std::cout << "saved to " << out_depth_filename << std::endl;
	}

}
