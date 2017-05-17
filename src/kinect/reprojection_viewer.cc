/*
#include "../lib/common.h"
#include "../lib/opencv.h"
#include "../lib/json.h"
#include "../lib/obj_img_correspondence.h"
#include "../lib/intrinsics.h"
#include "../lib/utility/misc.h"
#include "lib/live/viewer.h"
#include "lib/live/grabber.h"
#include "lib/live/checkerboard.h"
#include "lib/kinect_reprojection_parameters.h"
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


	grabber grab(grabber::color | grabber::depth);

	viewer view(754+754, 424+30);
	auto& min_d = view.add_slider("depth min ", 0, 20000);
	auto& max_d = view.add_slider("depth max", 6000, 20000);
	auto& diff_range = view.add_slider("difference range", 100, 500);
	auto& offset = view.add_slider("depth offset (-200 + ..) (mm)", 200, 400);
	auto& scaledown = view.add_slider("scaledown (%)", 30, 100);
	auto& superimpose = view.add_slider("superimpose (%)", 0, 100);
	
	depth_mode shown_depth_mode = depth_mode::reprojected;
	
	cv::Mat_<float> reprojected_depth(1080, 1920);
	cv::Mat_<float> z_buffer(1080, 1920);
	
	bool running = true;
	while(running) {
		int z_offset = -200 + offset.value;

		grab.grab();
		view.clear();
		
		cv::Mat_<cv::Vec3b> color = grab.get_color_frame();
		cv::Mat_<float> depth = grab.get_depth_frame();

		float scale = scaledown.value / 100.0;
		int scaled_w = 1920 * scale;
		int scaled_h = 1080 * scale;

		z_buffer.setTo(INFINITY);
		reprojected_depth.setTo(0);
		for(int dy = 0; dy < 424; ++dy) for(int dx = 0; dx < 512; ++dx) {
			float dz = depth(dy, dx);
			if(dz < 0.001) continue;			

			dz += z_offset;
			vec3 i_d(dx*dz, dy*dz, dz);
			vec3 v_d = reprojection_parameters.ir_intrinsic_inv * i_d;
			vec3 v_c = reprojection_parameters.rotation.t() * (v_d - reprojection_parameters.translation);
			vec3 i_c = reprojection_parameters.color_intrinsic * v_c;
			i_c /= i_c[2];
			
			int cx = scale * i_c[0];
			int cy = scale * i_c[1];
			float cz = v_c[2]; // !!!
			if(cx < 0 || cx >= scaled_w || cy < 0 || cy >= scaled_h) continue;
			
			float z;
			if(shown_depth_mode == depth_mode::original) z = depth(dy, dx);
			else if(shown_depth_mode == depth_mode::reprojected) z = cz;
			else z = depth(dy, dx) - cz;
			
			float& old_cz = z_buffer(cy, cx);
			if(cz > old_cz) continue;
			reprojected_depth(cy, cx) = z;
			old_cz = cz;
		}
		
		cv::Mat_<float> shown_reprojected_depth;
		cv::resize(cv::Mat(reprojected_depth, cv::Rect(0, 0, scaled_w, scaled_h)), shown_reprojected_depth, cv::Size(1920, 1090), cv::INTER_NEAREST);

		
		view.draw(cv::Rect(0, 0, 754, 424), color);
		cv::Rect depth_rect(754, 0, 754, 424);
		if(shown_depth_mode == depth_mode::difference) view.draw_depth(depth_rect, shown_reprojected_depth, -diff_range.value/2, +diff_range.value/2);
		else view.draw_depth(depth_rect, shown_reprojected_depth, min_d.value, max_d.value);
		if(superimpose.value > 0) {
			float blend = superimpose.value / 100.0;
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
	};
}
*/
int main(){}
