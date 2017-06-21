#include "../lib/args.h"
#include "../lib/viewer.h"
#include "../lib/image_io.h"
#include "../lib/filesystem.h"
#include "lib/live/grabber.h"
#include <format.h>
#include <iostream>

using namespace tlz;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "[snap_filename{}.png] [images/] [depths/] [ir/]");
	std::string filename_tpl = string_opt_arg("snap_{:04d}.png");
	std::string out_images_dir = out_dirname_opt_arg("snap/images/");
	std::string out_depths_dir = out_dirname_opt_arg("snap/depths/");
	std::string out_ir_dir = out_dirname_opt_arg("snap/ir/");
	
	grabber grab(grabber::color | grabber::depth | grabber::ir);

	viewer view(754+512+512, 424);
	auto& min_d = view.add_int_slider("depth min ", 0, 0, 20000);
	auto& max_d = view.add_int_slider("depth max", 6000, 0, 20000);
	auto& min_ir = view.add_int_slider("ir min", 0, 0x0000, 0xffff);
	auto& max_ir = view.add_int_slider("ir max", 0xffff, 0x0000, 0xffff);
	
	int snap_counter = 0;
	
	bool cont = true;
	while(cont) {
		grab.grab();
		view.clear();
		
		cv::Mat_<cv::Vec3b> image = grab.get_color_frame();
		cv::Mat_<ushort> ir = grab.get_original_ir_frame();
		cv::Mat_<real> depth = grab.get_depth_frame();
		
		view.draw(cv::Rect(0, 0, 754, 424), image);
		view.draw(cv::Rect(754, 0, 512, 424), viewer::visualize_ir(ir, min_ir, max_ir));
		view.draw_depth(cv::Rect(754+512, 0, 512, 424), depth, min_d, max_d);
		
		grab.release();
		
		int keycode;
		cont = view.show(keycode);
		if(keycode == enter_keycode) {
			std::cout << "taking snapshot " << snap_counter << std::endl;
			std::string out_filename = fmt::format(filename_tpl, snap_counter);
			snap_counter++;
			std::string out_image_filename = filename_append(out_images_dir, out_filename);
			std::string out_depth_filename = filename_append(out_depths_dir, out_filename);
			std::string out_ir_filename = filename_append(out_ir_dir, out_filename);
			save_texture(out_image_filename, image);
			save_depth(out_depth_filename, depth);
			save_ir(out_ir_filename, ir);
		}
	}
}
