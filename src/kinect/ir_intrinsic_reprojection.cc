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

const cv::Vec3b orig_color(0, 0, 255);
const cv::Vec3b reproj_color(0, 0, 255);

[[noreturn]] void usage_fail() {
	std::cout << "usage: ir_intrinsic_reprojection cols rows square_width ir_intr.json" << std::endl;
	std::exit(1);
}
int main(int argc, const char* argv[]) {
	if(argc <= 4) usage_fail();
	int cols = std::atoi(argv[1]);
	int rows = std::atoi(argv[2]);
	real square_width = std::atof(argv[3]);
	std::string ir_intrinsics_filename = argv[4];
	
	intrinsics ir_intr = decode_intrinsics(import_json_file(ir_intrinsics_filename));
	
	grabber grab(grabber::ir);

	viewer view(2*512, 2*424+30, true);
	auto& min_ir = view.add_slider("ir min", 0, 0xffff);
	auto& max_ir = view.add_slider("ir max", 0xffff, 0xffff);
	auto& exaggeration = view.add_slider("exaggeration (%)", 100, 1000);
			
	do {
		grab.grab();
		view.clear();
				
		cv::Mat_<uchar> ir = grab.get_ir_frame(min_ir.value, max_ir.value);

		cv::Mat_<cv::Vec3b> large_ir;
		{
			cv::Mat_<uchar> large_ir_;
			cv::resize(ir, large_ir_, cv::Size(2*512, 2*424), cv::INTER_NEAREST);
			cv::cvtColor(large_ir_, large_ir, CV_GRAY2BGR);
		}

		checkerboard ir_chk = detect_checkerboard(ir, cols, rows, square_width);

		std::vector<vec2> image_points = checkerboard_image_corners(ir_chk);
		std::vector<vec3> object_points = checkerboard_world_corners(cols, rows, square_width);

		real reprojection_error = NAN;
		
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
			
			// calculate distances (z in view space) for each point
			reprojection_error = 0.0;
			for(int idx = 0; idx < rows*cols; ++idx) {
				const vec2& i_orig = image_points[idx];
				const vec3& w = object_points[idx];
				
				vec3 v = rotation * w + translation;
				vec3 i_h = ir_intr.K * v;
				vec2 i_reproj(i_h[0] / i_h[2], i_h[1] / i_h[2]);
				
				vec2 diff = i_reproj - i_orig;
				reprojection_error += sq(diff[0]) + sq(diff[1]);
				
				vec2 viz_diff = (exaggeration.value/100.0)*diff;
				vec2 viz_i_reproj = i_orig + viz_diff;
					
				cv::circle(large_ir, cv::Point(2*i_orig[0], 2*i_orig[1]), 7, cv::Scalar(orig_color), 1);
				cv::circle(large_ir, cv::Point(2*viz_i_reproj[0], 2*viz_i_reproj[1]), 3, cv::Scalar(reproj_color), -1);
			}
			reprojection_error /= rows*cols;
			reprojection_error = std::sqrt(reprojection_error);
		}
		
		view.draw(cv::Rect(0, 0, 2*512, 2*424), large_ir);
		view.draw_text(cv::Rect(0, 2*424, 2*512, 30), "rms reprojection error: " + std::to_string(reprojection_error) + " pixel", viewer::center);
		grab.release();

	} while(view.show());
}

