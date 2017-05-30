#include "../lib/common.h"
#include "../lib/opencv.h"
#include "../lib/image_io.h"
#include "../lib/misc.h"
#include "../lib/obj_img_correspondence.h"
#include "lib/live/viewer.h"
#include "lib/live/grabber.h"
#include "lib/live/checkerboard.h"
#include <string>

using namespace tlz;

std::vector<checkerboard> color_chks, ir_chks;
std::string mode;
int cols;
int rows;
real square_width;
std::string out_cors_set_filename;


void save_correspondences_set() {
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
	
	export_json_file(j_cors_set, out_cors_set_filename);
}

[[noreturn]] void usage_fail() {
	std::cout << "usage: checkerboard_samples color/ir/both cols rows square_width out_cors_set.json [out_images_dir/] [restore_from_images]" << std::endl;
	std::exit(1);
}
int main(int argc, const char* argv[]) {
	if(argc <= 5) usage_fail();
	mode = argv[1];
	cols = std::atoi(argv[2]);
	rows = std::atoi(argv[3]);
	square_width = std::atof(argv[4]);
	out_cors_set_filename = argv[5];
	std::string out_images_dirname;
	if(argc > 6) out_images_dirname = argv[6];
	if(mode != "color" && mode != "ir" && mode != "both") usage_fail();
	bool restore_from_images = false;
	if(argc > 7) restore_from_images = (std::string(argv[7]) == "restore_from_images");

	bool autosave = false;

	
	if(restore_from_images) {
	
		int count = 0;
		std::cout << "restoring checkerboards from images" << std::endl;
		for(int i = 0; ; ++i) {
			checkerboard color_chk, ir_chk;
			if(mode == "color" || mode == "both") {
				std::string color_filename = out_images_dirname + "/color_" + std::to_string(i) + ".png";
				cv::Mat_<cv::Vec3b> color = cv::imread(color_filename, CV_LOAD_IMAGE_COLOR);
				if(color.empty()) break;
				color_chk = detect_color_checkerboard(color, cols, rows, square_width);
	
				//auto viz = visualize_checkerboard(color, color_chk);
				//cv::imshow("Color", viz);
				//cv::waitKey(1);
			}
			if(mode == "ir" || mode == "both") {
				std::string ir_filename = out_images_dirname + "/ir_" + std::to_string(i) + ".png";
				cv::Mat_<ushort> ir = cv::imread(ir_filename, CV_LOAD_IMAGE_ANYDEPTH);
				if(ir.empty()) break;
				ir_chk = detect_ir_checkerboard(ir, cols, rows, square_width);

				//auto viz = visualize_checkerboard(ir, ir_chk);
				//cv::imshow("IR", viz);
				//cv::waitKey(1);
			}

			if(mode == "color" && color_chk) {
				color_chks.push_back(color_chk); ++count;
				std::cout << "got color checkerboard " << count << std::endl;
			} else if(mode == "ir" && ir_chk) {
				ir_chks.push_back(ir_chk);  ++count;
				std::cout << "got ir checkerboard " << count << std::endl;
			} else if(mode == "both" && color_chk && ir_chk) {
				color_chks.push_back(color_chk);
				ir_chks.push_back(ir_chk); 
				 ++count;
				std::cout << "got color+ir checkerboards " << count << std::endl;
			}
		}
		
		std::cout << "restored " << count << " checkerboards, saving correspondences" << std::endl;
		save_correspondences_set();

	} else {
		
		grabber grab(grabber::color | grabber::ir);
	
		viewer view(754+512, 424+30);
		auto& min_ir = view.add_slider("ir min", 0, 0xffff);
		auto& max_ir = view.add_slider("ir max", 0xffff, 0xffff);
		
		
		int count = 0;
		
		std::cout << "running viewer... (enter to capture checkerboard, esc to end)" << std::endl; 
		bool running = true;
		while(running) {
			grab.grab();
			view.clear();
			
			checkerboard color_chk, ir_chk;
			
			cv::Mat_<cv::Vec3b> color = grab.get_color_frame();
			cv::Mat_<uchar> ir = grab.get_ir_frame(min_ir.value, max_ir.value);
			cv::Mat_<ushort> ir_orig = grab.get_original_ir_frame();
	
			if(mode == "color" || mode == "both") color_chk = detect_color_checkerboard(color, cols, rows, square_width);
			if(mode == "ir" || mode == "both") ir_chk = detect_ir_checkerboard(ir, cols, rows, square_width);
							
			view.draw(cv::Rect(0, 0, 754, 424), visualize_checkerboard(color, color_chk));
			view.draw(cv::Rect(754, 0, 512, 424), visualize_checkerboard(ir, ir_chk));
			grab.release();
			
	
			view.draw_text(cv::Rect(0, 424, 754+512-10, 30), std::to_string(count) + " samples collected", viewer::right);
	
			int keycode = 0;
			running = view.show(keycode);
			
			if(keycode == enter_keycode) {
				if(! out_images_dirname.empty()) {
					std::string color_filename = out_images_dirname + "/color_" + std::to_string(count) + ".png";
					std::string ir_filename = out_images_dirname + "/ir_" + std::to_string(count) + ".png";
					std::vector<int> params = { CV_IMWRITE_PNG_COMPRESSION, 0 };
					if(mode == "color" || mode == "both") cv::imwrite(color_filename, color, params);
					if(mode == "ir" || mode == "both") cv::imwrite(ir_filename, ir_orig, params);
				}
	
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
				
				if(autosave) {
					std::cout << "autosaving correspondences" << std::endl;
					save_correspondences_set();
				}
			}
		}
		
		if(! autosave) {
			std::cout << "now saving correspondences" << std::endl;
			save_correspondences_set();
		}
	
	}
		
	std::cout << "done" << std::endl;
}
