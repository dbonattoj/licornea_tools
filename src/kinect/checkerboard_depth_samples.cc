#include "../lib/common.h"
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

const real reprojection_error_max_threshold = 0.8;

[[noreturn]] void usage_fail() {
	std::cout << "usage: checkerboard_depth_samples cols rows out_chk_samples.json" << std::endl;
	std::exit(1);
}
int main(int argc, const char* argv[]) {
	if(argc <= 3) usage_fail();
	int cols = std::atoi(argv[1]);
	int rows = std::atoi(argv[2]);
	std::string out_chk_samples_filename = argv[3];
		
	grabber grab(grabber::depth | grabber::ir);

	viewer view(512+512, 424+30);
	auto& min_ir = view.add_int_slider("ir min", 0, 0x0000, 0xffff);
	auto& max_ir = view.add_int_slider("ir max", 0xffff, 0x0000, 0xffff);
	auto& min_d = view.add_int_slider("depth min ", 0, 0, 20000);
	auto& max_d = view.add_int_slider("depth max", 6000, 0, 20000);
	auto& granularity = view.add_int_slider("granularity", 2, 1, 30);
	
	bool autocollect = false;
	struct checkerboard_sample {
		std::vector<vec2> corners;
		std::vector<checkerboard_pixel_depth_sample> pixel_samples;
	};
	std::vector<checkerboard_sample> samples;
	int total_pixels = 0;
	
	checkerboard_sample current_sample;
	bool has_sample;
	
	std::cout << "running viewer... (enter to capture samples, esc to end)" << std::endl; 
	bool running = true;
	while(running) {
		grab.grab();
		view.clear();
				
		cv::Mat_<float> depth = grab.get_depth_frame();
		cv::Mat_<uchar> ir = grab.get_ir_frame(min_ir.value(), max_ir.value());

		has_sample = false;

		checkerboard ir_chk = detect_ir_checkerboard(ir, cols, rows, 0.0);
		if(ir_chk && granularity.value() > 0) {
			current_sample = checkerboard_sample();
			current_sample.corners = checkerboard_image_corners(ir_chk);
			current_sample.pixel_samples = checkerboard_pixel_depth_samples(ir_chk, depth, granularity.value());
			has_sample = true;
		}
				
		view.draw(cv::Rect(0, 0, 512, 424), visualize_checkerboard(ir, ir_chk));
		if(has_sample) view.draw(cv::Rect(512, 0, 512, 424), visualize_checkerboard_pixel_samples(view.visualize_depth(depth, min_d.value(), max_d.value()), current_sample.pixel_samples));
		else view.draw_depth(cv::Rect(512, 0, 512, 424), depth, min_d.value(), max_d.value());		
		
		view.draw_text(cv::Rect(20, 424, 512+512-20, 30), std::string("(a) autocollect is ") + (autocollect ? "ON" : "off"), viewer::left);
		view.draw_text(cv::Rect(0, 424, 512+512-20, 30), std::to_string(samples.size()) + " checkerboard samples (" + std::to_string(total_pixels) + " pixels total)", viewer::right);
		
		grab.release();

		int keycode = 0;
		running = view.show(keycode);
		
		bool collect = false;
		
		if(keycode == enter_keycode && ir_chk) collect = true;
		else if(keycode == 'a') autocollect = !autocollect;
		
		if(autocollect) collect = true;
		
		if(collect && has_sample) {
			samples.push_back(current_sample);
			total_pixels += current_sample.pixel_samples.size();
		}
	}
	
	
	std::cout << "saving collected checkerboard samples" << std::endl;
	{
		json j_chk_samples = json::array();
		for(const checkerboard_sample& chk : samples) {
			json j_chk_samp = json::object();
			
			json j_corners = json::array();
			for(const vec2& corner : chk.corners) {
				json j_corner = json::object();
				j_corner["x"] = corner[0];
				j_corner["y"] = corner[1];
				j_corners.push_back(j_corner);
			}
			j_chk_samp["corners"] = j_corners;
			
			json j_pixels = json::array();
			for(const checkerboard_pixel_depth_sample& pix : chk.pixel_samples) {
				json j_pixel = json::object();
				j_pixel["x"] = pix.coordinates[0];
				j_pixel["y"] = pix.coordinates[1];
				j_pixel["d"] = pix.measured_depth;
				j_pixels.push_back(j_pixel);
			}
			j_chk_samp["pixels"] = j_pixels;
			
			j_chk_samples.push_back(j_chk_samp);
		}
		export_json_file(j_chk_samples, out_chk_samples_filename);
	}
	
	std::cout << "done" << std::endl;
}
