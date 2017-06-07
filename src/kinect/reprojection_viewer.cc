#include "../lib/common.h"
#include "../lib/opencv.h"
#include "../lib/json.h"
#include "../lib/intrinsics.h"
#include "../lib/misc.h"
#include "../lib/viewer.h"
#include "lib/live/grabber.h"
#include "lib/live/checkerboard.h"
#include "lib/kinect_reprojection_parameters.h"
#include "lib/kinect_reprojection.h"
#include <string>
#include <cmath>

using namespace tlz;

enum class depth_mode {
	original,
	reprojected,
	difference
};

[[noreturn]] void usage_fail() {
	std::cout << "usage: reprojection_viewer reprojection.json" << std::endl;
	std::exit(1);
}
int main(int argc, const char* argv[]) {
	if(argc <= 1) usage_fail();
	std::string reprojection_parameters_filename = argv[1];

	std::cout << "loading reprojection parameters" << std::endl;
	kinect_reprojection_parameters reprojection_parameters = decode_kinect_reprojection_parameters(import_json_file(reprojection_parameters_filename));
	kinect_reprojection reprojection(reprojection_parameters);

	grabber grab(grabber::color | grabber::depth);

	viewer view(754+754, 424+30);
	auto& min_d = view.add_int_slider("depth min ", 0, 0, 20000);
	auto& max_d = view.add_int_slider("depth max", 6000, 0, 20000);
	auto& diff_range = view.add_int_slider("difference range", 100, 0, 500);
	auto& scaledown = view.add_int_slider("scaledown (%)", 30, 10, 100);
	auto& superimpose = view.add_int_slider("superimpose (%)", 0, 0, 100);

	depth_mode shown_depth_mode = depth_mode::reprojected;

	cv::Mat_<float> reprojected_depth(1080, 1920);
	cv::Mat_<float> ir_z_buffer(1080, 1920);

	bool running = true;
	while(running) {
		float scale = scaledown.value() / 100.0;
		int scaled_w = 1920 * scale;
		int scaled_h = 1080 * scale;

		grab.grab();
		view.clear();

		cv::Mat_<cv::Vec3b> color = grab.get_color_frame();
		cv::Mat_<float> depth = grab.get_depth_frame();

		auto samples = reprojection.reproject_ir_to_color_samples(depth, depth, true);

		ir_z_buffer.setTo(INFINITY);
		reprojected_depth.setTo(0.0);
		for(const auto& samp : samples) {
			int scx = scale * samp.color_coordinates[0], scy = scale * samp.color_coordinates[1];
			if(scx < 0 || scx >= scaled_w || scy < 0 || scy >= scaled_h) continue;

			real d;
			if(shown_depth_mode == depth_mode::original) d = samp.ir_depth;
			else if(shown_depth_mode == depth_mode::reprojected) d = samp.color_depth;
			else d = samp.ir_depth - samp.color_depth;
			
			float& old_ir_z = ir_z_buffer(scy, scx);
			if(samp.ir_depth > old_ir_z) continue;
			reprojected_depth(scy, scx) = d;
			old_ir_z = samp.ir_depth;
		}
		
		cv::Mat_<float> shown_reprojected_depth;
		cv::resize(cv::Mat(reprojected_depth, cv::Rect(0, 0, scaled_w, scaled_h)), shown_reprojected_depth, cv::Size(1920, 1090), cv::INTER_NEAREST);
		
		view.draw(cv::Rect(0, 0, 754, 424), color);
		cv::Rect depth_rect(754, 0, 754, 424);
		if(shown_depth_mode == depth_mode::difference) view.draw_depth(depth_rect, shown_reprojected_depth, -diff_range.value()/2, +diff_range.value()/2);
		else view.draw_depth(depth_rect, shown_reprojected_depth, min_d.value(), max_d.value());
		if(superimpose.value() > 0) {
			float blend = superimpose.value() / 100.0;
			view.draw(depth_rect, color, blend);
		}


		int label_w = 250;
		cv::Vec3b selected_mode_color(0, 255, 200);
		auto col = [&](depth_mode md) { return (md == shown_depth_mode ? selected_mode_color : view.text_color); };
		view.draw_text(cv::Rect(0, 424, 754, 30), "color image", viewer::center);
		view.draw_text(cv::Rect(754+40, 424, label_w, 30), "(o) original depth", viewer::left, col(depth_mode::original));
		view.draw_text(cv::Rect(754+40+label_w, 424, label_w, 30), "(r) reprojected depth", viewer::left, col(depth_mode::reprojected));
		view.draw_text(cv::Rect(754+40+2*label_w, 424, label_w, 30), "(d) difference", viewer::left, col(depth_mode::difference));


		grab.release();
		
		int keycode;
		running = view.show(keycode);
		
		if(keycode == 'o') shown_depth_mode = depth_mode::original;
		else if(keycode == 'r') shown_depth_mode = depth_mode::reprojected;
		else if(keycode == 'd') shown_depth_mode = depth_mode::difference;
	}
}
