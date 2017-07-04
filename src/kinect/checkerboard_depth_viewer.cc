#include "../lib/common.h"
#include "../lib/args.h"
#include "../lib/opencv.h"
#include "../lib/intrinsics.h"
#include "../lib/misc.h"
#include "../lib/obj_img_correspondence.h"
#include "../lib/viewer.h"
#include "lib/live/grabber.h"
#include "lib/live/checkerboard.h"
#include <string>
#include <cassert>
#include <fstream>

using namespace tlz;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "cols rows square_width ir_intr.json");
	int cols = int_arg();
	int rows = int_arg();
	real square_width = real_arg();
	intrinsics ir_intr = intrinsics_arg();
		
	grabber grab(grabber::depth | grabber::ir);

	viewer view(512+512, 424+2*30);
	auto& min_ir = view.add_int_slider("ir min", 0, 0x0000, 0xffff);
	auto& max_ir = view.add_int_slider("ir max", 0xffff, 0x0000, 0xffff);
	auto& min_d = view.add_int_slider("depth min ", 0, 0, 20000);
	auto& max_d = view.add_int_slider("depth max", 6000, 0, 20000);
	auto& granularity = view.add_int_slider("granularity", 2, 1, 30);
		
	std::cout << "running viewer... (esc to end)" << std::endl; 
	bool running = true;
	while(running) {
		grab.grab();
		view.clear();
				
		cv::Mat_<float> depth = grab.get_depth_frame();
		cv::Mat_<uchar> ir = grab.get_ir_frame(min_ir.value(), max_ir.value());

		checkerboard ir_chk = detect_ir_checkerboard(ir, cols, rows, square_width);

		real avg_calculated_depth = NAN, avg_measured_depth = NAN;
		real rms_depth_error = NAN, reprojection_error = NAN;
		int count = 0;
		
		std::vector<checkerboard_pixel_depth_sample> pixel_depths;
		if(ir_chk && granularity.value() > 0) {		
			pixel_depths = checkerboard_pixel_depth_samples(ir_chk, depth, granularity.value());
			checkerboard_extrinsics ext = estimate_checkerboard_extrinsics(ir_chk, ir_intr);
			calculate_checkerboard_pixel_depths(ir_intr, ext, pixel_depths);
			count = pixel_depths.size();
			
			avg_calculated_depth = 0.0, avg_measured_depth = 0.0; rms_depth_error = 0.0;
			for(const auto& samp : pixel_depths) {
				real err = samp.calculated_depth - samp.measured_depth;
				if(std::abs(err) > 80) { count--; continue; }
				rms_depth_error += sq(err);
				avg_calculated_depth += samp.calculated_depth;
				avg_measured_depth += samp.measured_depth;
			}
			rms_depth_error /= count;
			rms_depth_error = std::sqrt(rms_depth_error);
			avg_calculated_depth /= count;
			avg_measured_depth /= count;
		}
		
		view.draw(cv::Rect(0, 0, 512, 424), visualize_checkerboard(ir, ir_chk));
		view.draw(cv::Rect(512, 0, 512, 424), visualize_checkerboard_pixel_samples(view.visualize_depth(depth, min_d.value(), max_d.value()), pixel_depths));

		view.draw_text(cv::Rect(20, 424, 512+512, 30), "avg calculated: " + std::to_string(avg_calculated_depth) + " mm", viewer::left);
		view.draw_text(cv::Rect(20+350, 424, 512+512, 30), "avg measured: " + std::to_string(avg_measured_depth) + " mm", viewer::left);
		//view.draw_text(cv::Rect(20+700, 424, 512+512, 30), "difference: " + std::to_string(avg_measured_depth-avg_calculated_depth) + " mm", viewer::left);
		view.draw_text(cv::Rect(20+700, 424, 512+512, 30), "ratio: " + std::to_string(avg_measured_depth/avg_calculated_depth), viewer::left);
		
		view.draw_text(cv::Rect(20, 424+30, 512+512-10, 30), "rms depth error: " + std::to_string(rms_depth_error) + " mm", viewer::left);
		view.draw_text(cv::Rect(20+430, 424+30, 512+512-10, 30), "count: " + std::to_string(count), viewer::left);
		view.draw_text(cv::Rect(20+570, 424+30, 512+512-10, 30), "reprojection err: " + std::to_string(reprojection_error), viewer::left);
		
		grab.release();

		running = view.show();
	}
	
	std::cout << "done" << std::endl;
}
