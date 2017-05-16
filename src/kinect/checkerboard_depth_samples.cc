#include "../lib/common.h"
#include "../lib/opencv.h"
#include "../lib/intrinsics.h"
#include "../lib/utility/misc.h"
#include "../lib/obj_img_correspondence.h"
#include "lib/live/viewer.h"
#include "lib/live/grabber.h"
#include "lib/live/checkerboard.h"
#include <string>
#include <cassert>
#include <fstream>

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: checkerboard_depth_samples cols rows square_width ir_intr.json out_samples.txt" << std::endl;
	std::exit(1);
}
int main(int argc, const char* argv[]) {
	if(argc <= 5) usage_fail();
	int cols = std::atoi(argv[1]);
	int rows = std::atoi(argv[2]);
	real square_width = std::atof(argv[3]);
	std::string ir_intrinsics_filename = argv[4];
	std::string point_samples_filename = argv[5];
	
	intrinsics ir_intr = decode_intrinsics(import_json_file(ir_intrinsics_filename));
	
	grabber grab(grabber::depth | grabber::ir);

	viewer view(512+512, 424+60);
	auto& min_ir = view.add_slider("ir min", 0, 0xffff);
	auto& max_ir = view.add_slider("ir max", 0xffff, 0xffff);
	auto& min_d = view.add_slider("depth min ", 0, 20000);
	auto& max_d = view.add_slider("depth max", 6000, 20000);
		
	struct sample {
		real x;
		real y;
		real measured_depth;
		real projected_depth;
	};
	std::vector<sample> samples;
	
	std::cout << "running viewer... (enter to capture samples, esc to end)" << std::endl; 
	bool running = true;
	while(running) {
	
		grab.grab();
		view.clear();
				
		cv::Mat_<float> depth = grab.get_depth_frame();
		cv::Mat_<uchar> ir = grab.get_ir_frame(min_ir.value, max_ir.value);

		checkerboard ir_chk = detect_checkerboard(ir, cols, rows, square_width);

		std::vector<vec2> image_points = checkerboard_image_corners(ir_chk);
		std::vector<vec3> object_points = checkerboard_world_corners(cols, rows, square_width);

		std::vector<real> projection_depths, measured_depths;
		real avg_error = NAN, stddev_error = NAN;
		int count = 0;

		if(ir_chk) {
			// calculate extrinsic	
			vec3 rotation_vec, translation;
			mat33 rotation;
			cv::solvePnP(
				object_points,
				image_points,
				ir_intr.K,
				ir_intr.distortion.cv_coeffs(),
				rotation_vec,
				translation,
				false
			);
			cv::Rodrigues(rotation_vec, rotation);
			
			// calculate distances (z in view space) for each corner
			for(const vec3& w : object_points) {
				projection_depths.push_back(0.0);
				real& projected_d = projection_depths.back();

				vec3 v = rotation * w + translation;
				projected_d = v[2];				
			}
			
			// get measured distance for each corner
			for(const vec2& i : image_points) {
				measured_depths.push_back(0.0);
				real& measured_d = measured_depths.back();
				
				int dx = i[0], dy = i[1];
				if(dx < 0 || dx >= 512 || dy < 0 || dy >= 424) continue;
				
				float dz = depth(dy, dx);
				if(dz < 0.001) continue;
				
				measured_d = dz;
			}
			
			
			// calculate avg and stddev error
			std::vector<real> errs;
			assert(projection_depths.size() == measured_depths.size());
			count = projection_depths.size();
			avg_error = 0.0; stddev_error = 0.0;
			for(int i = 0; i < projection_depths.size(); ++i) {
				if(measured_depths[i] == 0.0 || projection_depths[i] == 0.0) {
					--count;
					continue;
				}
				
				real err = measured_depths[i] - projection_depths[i];
				errs.push_back(err);
				avg_error += err;
			}
			avg_error /= count;
			for(real err : errs) stddev_error += sq(err - avg_error);
			stddev_error /= count;
			stddev_error = std::sqrt(stddev_error);
		}
		
		view.draw(cv::Rect(0, 0, 512, 424), visualize_checkerboard(ir, ir_chk));
		view.draw_depth(cv::Rect(512, 0, 512, 424), depth, min_d.value, max_d.value);		
		if(ir_chk) {
			view.draw_text(cv::Rect(0, 424, 512+512-10, 30), "z offset avg: " + std::to_string(avg_error) + ", stddev: " + std::to_string(stddev_error) + ", count: " + std::to_string(count), viewer::left);
		}
		view.draw_text(cv::Rect(0, 424+30, 512+512-10, 30), std::to_string(samples.size()) + " point samples collected", viewer::right);
		grab.release();

		int keycode = 0;
		running = view.show(keycode);
		
		if(keycode == viewer::enter_keycode) {
			std::cout << "collecting " << count << " point samples" << std::endl; 
			for(int i = 0; i < projection_depths.size(); ++i) {
				if(measured_depths[i] == 0.0 || projection_depths[i] == 0.0) continue;
				vec2 image_point = image_points[i];
				sample s;
				s.x = image_point[0];
				s.y = image_point[1];
				s.measured_depth = measured_depths[i];
				s.projected_depth = projection_depths[i];
				samples.push_back(s);
			}
		}
	}
	
	
	std::cout << "saving collected point samples" << std::endl;
	{
		std::ofstream stream(point_samples_filename);
		stream << "x y measured projected difference\n";
		for(const sample& samp : samples)
			stream << samp.x << " " << samp.y << " " << samp.measured_depth << " " << samp.projected_depth << samp.measured_depth-samp.projected_depth << "\n";
	}
	
	std::cout << "done" << std::endl;
}
