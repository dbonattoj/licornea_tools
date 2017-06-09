#include "../lib/common.h"
#include "../lib/args.h"
#include "../lib/opencv.h"
#include "../lib/intrinsics.h"
#include "../lib/misc.h"
#include "../lib/viewer.h"
#include "lib/live/grabber.h"
#include "lib/live/checkerboard.h"
#include "lib/kinect_reprojection.h"
#include "lib/densify/depth_densify.h"
#include <string>
#include <cassert>
#include <fstream>

using namespace tlz;

enum class depth_mode {
	original,
	reprojected
};

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "cols rows square_width reprojection.json");
	int cols = int_arg();
	int rows = int_arg();
	real square_width = real_arg();
	std::string reprojection_parameters_filename = in_filename_arg();
	
	std::cout << "loading reprojection parameters" << std::endl;
	kinect_reprojection_parameters reprojection_parameters = decode_kinect_reprojection_parameters(import_json_file(reprojection_parameters_filename));
	const intrinsics& color_intr = reprojection_parameters.color_intrinsics;
	kinect_reprojection reprojection(reprojection_parameters);
	
	grabber grab(grabber::depth | grabber::color);

	viewer view(754+754, 424+30+30);
	auto& min_d = view.add_int_slider("depth min ", 0, 0, 20000);
	auto& max_d = view.add_int_slider("depth max", 6000, 0, 20000);
	auto& granularity = view.add_int_slider("granularity", 15, 1, 60);
	auto& superimpose = view.add_int_slider("superimpose (%)", 0, 0, 100);
		
	depth_mode used_depth_mode = depth_mode::reprojected;

	auto densifier = make_depth_densify("fast");
	cv::Mat_<real> reprojected_depth(1080, 1920);

	std::cout << "running viewer... (esc to end)" << std::endl; 
	bool running = true;
	while(running) {
		grab.grab();
		view.clear();
				
		cv::Mat_<cv::Vec3b> color = grab.get_color_frame();
		cv::Mat_<real> depth = grab.get_depth_frame();
		
		// reproject depth
		auto samples = reprojection.reproject_ir_to_color_samples(depth, depth, true);
		if(used_depth_mode == depth_mode::original)
			for(auto& sample : samples) sample.color_depth = sample.ir_depth;
		
		densifier->densify(samples, reprojected_depth);

		// detect checkerboard, compare calculated&measured(+reprojected) depth
		checkerboard color_chk = detect_color_checkerboard(color, cols, rows, square_width);

		real avg_calculated_depth = NAN, avg_measured_depth = NAN;
		real rms_depth_error = NAN, reprojection_error = NAN;
		int count = 0;
		
		std::vector<checkerboard_pixel_depth_sample> pixel_depths;
		if(color_chk && granularity.value() > 0) {		
			pixel_depths = checkerboard_pixel_depth_samples(color_chk, reprojected_depth, granularity.value());
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
		view.draw(depth_rect, visualize_checkerboard_pixel_samples(view.visualize_depth(reprojected_depth, min_d.value(), max_d.value()), pixel_depths, 4));

		if(superimpose.value() > 0) {
			float blend = superimpose.value() / 100.0;
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

