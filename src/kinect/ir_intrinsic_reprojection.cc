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

const cv::Vec3b orig_color(0, 0, 255);
const cv::Vec3b reproj_color(0, 0, 255);


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "cols rows square_width ir_intr.json");
	int cols = int_arg();
	int rows = int_arg();
	real square_width = real_arg();
	intrinsics ir_intr = intrinsics_arg();
	
	grabber grab(grabber::ir);

	viewer view(2*512, 2*424+30, true);
	auto& min_ir = view.add_int_slider("ir min", 0, 0x0000, 0xffff);
	auto& max_ir = view.add_int_slider("ir max", 0xffff, 0x0000, 0xffff);
	auto& exaggeration = view.add_int_slider("exaggeration (%)", 100, 100, 1000);
			
	do {
		grab.grab();
		view.clear();
				
		cv::Mat_<uchar> ir = grab.get_ir_frame(min_ir.value(), max_ir.value());

		cv::Mat_<cv::Vec3b> large_ir;
		{
			cv::Mat_<uchar> large_ir_;
			cv::resize(ir, large_ir_, cv::Size(2*512, 2*424), cv::INTER_NEAREST);
			cv::cvtColor(large_ir_, large_ir, CV_GRAY2BGR);
		}

		checkerboard ir_chk = detect_ir_checkerboard(ir, cols, rows, square_width);

		std::vector<vec2> image_points = checkerboard_image_corners(ir_chk);
		std::vector<vec3> object_points = checkerboard_world_corners(cols, rows, square_width);

		real reprojection_error = NAN;
		
		if(ir_chk) {
			// calculate extrinsic, and reproject
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

			std::vector<vec2> reprojected_image_points;
			cv::projectPoints(
				object_points,
				rotation_vec,
				translation,
				ir_intr.K,
				ir_intr.distortion.cv_coeffs(),
				reprojected_image_points
			);

			
			// calculate distances between original and reprojected point
			reprojection_error = 0.0;
			for(int idx = 0; idx < rows*cols; ++idx) {
				const vec2& i_orig = image_points[idx];
				const vec2& i_reproj = reprojected_image_points[idx];
				
				vec2 diff = i_reproj - i_orig;
				reprojection_error += sq(diff[0]) + sq(diff[1]);
				
				vec2 viz_diff = (exaggeration.value()/100.0)*diff;
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

