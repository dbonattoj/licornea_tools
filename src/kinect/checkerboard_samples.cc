#include "../lib/common.h"
#include "../lib/opencv.h"
#include "../lib/obj_img_correspondence.h"
#include "lib/live/viewer.h"
#include "lib/live/grabber.h"
#include "lib/live/checkerboard.h"
#include <string>

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: checkerboard_samples color/ir/both cols rows square_width out_cors_set.json" << std::endl;
	std::exit(1);
}
int main(int argc, const char* argv[]) {
	if(argc <= 5) usage_fail();
	std::string mode = argv[1];
	int cols = std::atoi(argv[2]);
	int rows = std::atoi(argv[3]);
	real square_width = std::atof(argv[4]);
	std::string out_cors_set_filename = argv[5];
	if(mode != "color" && mode != "ir" && mode != "both") usage_fail();
	
	std::string input_filename = argv[1];
	std::string output_filename = argv[2];
	std::string intrinsics_filename = argv[3];
	
	grabber grab(grabber::color | grabber::ir);

	viewer view(754+512, 424+30);
	auto& min_ir = view.add_slider("ir min", 0, 0xffff);
	auto& max_ir = view.add_slider("ir max", 0xffff, 0xffff);
	
	std::vector<checkerboard> color_chks, ir_chks;
	int count = 0;
	
	std::cout << "running viewer... (enter to capture checkerboard, esc to end)" << std::endl; 
	bool running = true;
	while(running) {
		grab.grab();
		view.clear();
		
		checkerboard color_chk, ir_chk;
		
		cv::Mat_<cv::Vec3b> color = grab.get_color_frame();
		cv::Mat_<uchar> ir = grab.get_ir_frame(min_ir.value, max_ir.value);

		if(mode == "color" || mode == "both") color_chk = detect_color_checkerboard(color, cols, rows, square_width);
		if(mode == "ir" || mode == "both") ir_chk = detect_ir_checkerboard(ir, cols, rows, square_width);
						
		view.draw(cv::Rect(0, 0, 754, 424), visualize_checkerboard(color, color_chk));
		view.draw(cv::Rect(754, 0, 512, 424), visualize_checkerboard(ir, ir_chk));
		grab.release();
		

		view.draw_text(cv::Rect(0, 424, 754+512-10, 30), std::to_string(count) + " samples collected", viewer::right);

		int keycode = 0;
		running = view.show(keycode);
		
		if(keycode == viewer::enter_keycode) {
			if(mode == "color" && color_chk) {
				color_chks.push_back(color_chk); ++count;
				std::cout << "recorded color checkerboard " << count << std::endl;
			} else if(mode == "ir" && ir_chk) {
				ir_chks.push_back(ir_chk);  ++count;
				std::cout << "recorded ir checkerboard " << count << std::endl;
			} else if(mode == "both" && color_chk && ir_chk) {
				color_chks.push_back(color_chk);
				ir_chks.push_back(ir_chk); 
				 ++count;
				std::cout << "recorded color+ir checkerboards " << count << std::endl;
			}
		}
	}
	
	std::cout << "assembling obj img correspondences set" << std::endl;
	json j_cors_set;
	std::vector<vec3> world_points = checkerboard_world_corners(cols, rows, square_width);
	if(mode == "color") {
		obj_img_correspondences_set<1, 1> set;
		for(const checkerboard& chk : color_chks)
			set.push_back(checkerboard_obj_img_correspondences(chk));
		j_cors_set = encode_obj_img_correspondences_set(set);
		
	} else if(mode == "ir") {
		obj_img_correspondences_set<1, 1> set;
		for(const checkerboard& chk : ir_chks)
			set.push_back(checkerboard_obj_img_correspondences(chk));
		j_cors_set = encode_obj_img_correspondences_set(set);
		
	} else if(mode == "both") {
		obj_img_correspondences_set<1, 2> set;
		for(auto it1 = color_chks.begin(), it2 = ir_chks.begin(); it1 != color_chks.end(); ++it1, ++it2)
			set.push_back(checkerboard_obj_2img_correspondences(*it1, *it2));
		j_cors_set = encode_obj_img_correspondences_set(set);
	}
	
	std::cout << "saving correspondences set" << std::endl;
	export_json_file(j_cors_set, out_cors_set_filename);
	
	std::cout << "done" << std::endl;
}
