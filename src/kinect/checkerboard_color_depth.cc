#include "../lib/common.h"
#include "../lib/opencv.h"
#include "../lib/intrinsics.h"
#include "../lib/utility/misc.h"
#include "lib/live/viewer.h"
#include "lib/live/grabber.h"
#include "lib/live/checkerboard.h"
#include "lib/kinect_reprojection.h"
#include <string>
#include <cassert>
#include <fstream>

using namespace tlz;

enum class depth_mode {
	original,
	reprojected
};

[[noreturn]] void usage_fail() {
	std::cout << "usage: checkerboard_color_depth cols rows square_width reprojection.json" << std::endl;
	std::exit(1);
}
int main(int argc, const char* argv[]) {
	if(argc <= 4) usage_fail();
	int cols = std::atoi(argv[1]);
	int rows = std::atoi(argv[2]);
	real square_width = std::atof(argv[3]);
	std::string reprojection_parameters_filename = argv[4];
	
	std::cout << "loading reprojection parameters" << std::endl;
	kinect_reprojection_parameters reprojection_parameters = decode_kinect_reprojection_parameters(import_json_file(reprojection_parameters_filename));
	const intrinsics& color_intr = reprojection_parameters.color_intrinsics;
	kinect_reprojection reprojection(reprojection_parameters);
	
	grabber grab(grabber::depth | grabber::color);

	viewer view(754+754, 424+30+30);
	auto& min_d = view.add_slider("depth min ", 0, 20000);
	auto& max_d = view.add_slider("depth max", 6000, 20000);
	auto& granularity = view.add_slider("granularity", 15, 60);
	auto& scaledown = view.add_slider("scaledown (%)", 30, 100);
	auto& superimpose = view.add_slider("superimpose (%)", 0, 100);
		
	depth_mode used_depth_mode = depth_mode::reprojected;

	cv::Mat_<float> reprojected_depth(1080, 1920);
	cv::Mat_<float> ir_z_buffer(1080, 1920);

	std::cout << "running viewer... (esc to end)" << std::endl; 
	bool running = true;
	while(running) {
		float scale = scaledown.value / 100.0;
		int scaled_w = 1920 * scale;
		int scaled_h = 1080 * scale;

		grab.grab();
		view.clear();
				
		cv::Mat_<cv::Vec3b> color = grab.get_color_frame();
		cv::Mat_<float> depth_ = grab.get_depth_frame();
		
		cv::Mat_<float> depth = depth_;

		// reproject depth
		auto samples = reprojection.reproject_ir_to_color_samples(depth, depth, true);

		ir_z_buffer.setTo(INFINITY);
		reprojected_depth.setTo(0.0);
		for(const auto& samp : samples) {
			int scx = scale * samp.color_coordinates[0], scy = scale * samp.color_coordinates[1];
			if(scx < 0 || scx >= scaled_w || scy < 0 || scy >= scaled_h) continue;

			real d;
			if(used_depth_mode == depth_mode::original) d = samp.ir_depth;
			else if(used_depth_mode == depth_mode::reprojected) d = samp.color_depth;
			
			float& old_ir_z = ir_z_buffer(scy, scx);
			if(samp.ir_depth > old_ir_z) continue;
			reprojected_depth(scy, scx) = d;
			old_ir_z = samp.ir_depth;
		}
		cv::Mat_<float> used_reprojected_depth;
		cv::resize(cv::Mat(reprojected_depth, cv::Rect(0, 0, scaled_w, scaled_h)), used_reprojected_depth, cv::Size(1920, 1090), cv::INTER_NEAREST);


		// detect checkerboard, compare calculated&measured(+reprojected) depth
		checkerboard color_chk = detect_color_checkerboard(color, cols, rows, square_width);

		real avg_calculated_depth = NAN, avg_measured_depth = NAN;
		real rms_depth_error = NAN, reprojection_error = NAN;
		int count = 0;
		
		std::vector<checkerboard_pixel_depth_sample> pixel_depths;
		if(color_chk && granularity.value > 0) {		
			pixel_depths = checkerboard_pixel_depth_samples(color_chk, used_reprojected_depth, granularity.value);
			checkerboard_extrinsics ext = estimate_checkerboard_extrinsics(color_chk, color_intr);
			calculate_checkerboard_pixel_depths(color_intr, ext, pixel_depths);
			count = pixel_depths.size();
			
			avg_calculated_depth = 0.0, avg_measured_depth = 0.0; rms_depth_error = 0.0;
			for(const auto& samp : pixel_depths) {
				real err = samp.calculated_depth - samp.measured_depth;
				rms_depth_error += sq(err);
				avg_calculated_depth += samp.calculated_depth;
				avg_measured_depth += samp.measured_depth;
			}
			rms_depth_error /= count;
			rms_depth_error = std::sqrt(rms_depth_error);
			avg_calculated_depth /= count;
			avg_measured_depth /= count;
		}

		

		// draw image & depth
		cv::Mat_<cv::Vec3b> shown_color = visualize_checkerboard(color, color_chk);
		view.draw(cv::Rect(0, 30, 754, 424), shown_color);

		cv::Rect depth_rect(754, 30, 754, 424);
		view.draw(depth_rect, visualize_checkerboard_pixel_samples(view.visualize_depth(used_reprojected_depth, min_d.value, max_d.value), pixel_depths, 4));

		if(superimpose.value > 0) {
			float blend = superimpose.value / 100.0;
			view.draw(depth_rect, shown_color, blend);
		}


		// draw labels
		int label_w = 250;
		cv::Vec3b selected_mode_color(0, 255, 200);
		auto col = [&](depth_mode md) { return (md == used_depth_mode ? selected_mode_color : view.text_color); };
		view.draw_text(cv::Rect(0, 0, 754, 30), "color image", viewer::center);
		view.draw_text(cv::Rect(754+100, 0, label_w, 30), "(o) original depth", viewer::left, col(depth_mode::original));
		view.draw_text(cv::Rect(754+100+label_w, 0, label_w, 30), "(r) reprojected depth", viewer::left, col(depth_mode::reprojected));


		view.draw_text(cv::Rect(20, 424+30, 754+754, 30), "avg calculated: " + std::to_string(avg_calculated_depth) + " mm", viewer::left);
		view.draw_text(cv::Rect(20+500, 424+30, 754+754, 30), "avg measured: " + std::to_string(avg_measured_depth) + " mm", viewer::left);
		view.draw_text(cv::Rect(20+1100, 424+30, 754+754, 30), "difference: " + std::to_string(avg_measured_depth-avg_calculated_depth) + " mm", viewer::left);
		
		grab.release();

		int keycode;
		running = view.show(keycode);
		
		if(keycode == 'o') used_depth_mode = depth_mode::original;
		else if(keycode == 'r') used_depth_mode = depth_mode::reprojected;
	}
	
	std::cout << "done" << std::endl;
}

