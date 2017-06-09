#include "../lib/common.h"
#include "../lib/args.h"
#include "../lib/opencv.h"
#include "../lib/json.h"
#include "../lib/intrinsics.h"
#include "../lib/misc.h"
#include "../lib/viewer.h"
#include "lib/live/grabber.h"
#include "lib/live/checkerboard.h"
#include "lib/kinect_reprojection_parameters.h"
#include "lib/kinect_reprojection.h"
#include "lib/densify/depth_densify.h"
#include <string>
#include <cmath>

using namespace tlz;

enum class depth_mode {
	original,
	reprojected,
	difference
};

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "reprojection.json [densify_mode=fast]");
	std::string reprojection_parameters_filename = in_filename_arg();
	std::string densify_mode = string_opt_arg("fast");

	std::cout << "loading reprojection parameters" << std::endl;
	kinect_reprojection_parameters reprojection_parameters = decode_kinect_reprojection_parameters(import_json_file(reprojection_parameters_filename));
	kinect_reprojection reprojection(reprojection_parameters);

	grabber grab(grabber::color | grabber::depth);

	viewer view(754+754, 424+30);
	auto& min_d = view.add_int_slider("depth min ", 0, 0, 20000);
	auto& max_d = view.add_int_slider("depth max", 6000, 0, 20000);
	auto& diff_range = view.add_int_slider("difference range", 100, 0, 500);
	auto& superimpose = view.add_int_slider("superimpose (%)", 0, 0, 100);

	depth_mode used_depth_mode = depth_mode::reprojected;

	auto densifier = make_depth_densify(densify_mode);
	cv::Mat_<real> reprojected_depth;
	
	bool running = true;
	while(running) {
		grab.grab();
		view.clear();

		cv::Mat_<cv::Vec3b> color = grab.get_color_frame();
		cv::Mat_<real> depth = grab.get_depth_frame();

		auto samples = reprojection.reproject_ir_to_color_samples(depth, depth, true);
		if(used_depth_mode == depth_mode::original)
			for(auto& samp : samples) samp.color_depth = samp.ir_depth;
		else if(used_depth_mode == depth_mode::difference)
			for(auto& samp : samples) samp.color_depth = samp.ir_depth - samp.color_depth;

		densifier->densify(samples, reprojected_depth);

		view.draw(cv::Rect(0, 0, 754, 424), color);
		cv::Rect depth_rect(754, 0, 754, 424);
		if(used_depth_mode == depth_mode::difference) view.draw_depth(depth_rect, reprojected_depth, -diff_range.value()/2, +diff_range.value()/2);
		else view.draw_depth(depth_rect, reprojected_depth, min_d.value(), max_d.value());
		if(superimpose.value() > 0) {
			float blend = superimpose.value() / 100.0;
			view.draw(depth_rect, color, blend);
		}


		int label_w = 250;
		cv::Vec3b selected_mode_color(0, 255, 200);
		auto col = [&](depth_mode md) { return (md == used_depth_mode ? selected_mode_color : view.text_color); };
		view.draw_text(cv::Rect(0, 424, 754, 30), "color image", viewer::center);
		view.draw_text(cv::Rect(754+40, 424, label_w, 30), "(o) original depth", viewer::left, col(depth_mode::original));
		view.draw_text(cv::Rect(754+40+label_w, 424, label_w, 30), "(r) reprojected depth", viewer::left, col(depth_mode::reprojected));
		view.draw_text(cv::Rect(754+40+2*label_w, 424, label_w, 30), "(d) difference", viewer::left, col(depth_mode::difference));


		grab.release();
		
		int keycode;
		running = view.show(keycode);
		
		if(keycode == 'o') used_depth_mode = depth_mode::original;
		else if(keycode == 'r') used_depth_mode = depth_mode::reprojected;
		else if(keycode == 'd') used_depth_mode = depth_mode::difference;
	}
}
