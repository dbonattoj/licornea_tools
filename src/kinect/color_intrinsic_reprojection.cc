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
	get_args(argc, argv, "cols rows square_width color_intr.json");
	int cols = int_arg();
	int rows = int_arg();
	real square_width = real_arg();
	intrinsics color_intr = intrinsics_arg();
		
	grabber grab(grabber::color);

	viewer view(1920, 1080+30, true);
	auto& exaggeration = view.add_int_slider("exaggeration (%)", 100, 100, 3000);
			
	do {
		grab.grab();
		view.clear();
				
		cv::Mat_<cv::Vec3b> color = grab.get_color_frame();

		checkerboard color_chk = detect_color_checkerboard(color, cols, rows, square_width);

		std::vector<vec2> image_points = checkerboard_image_corners(color_chk);
		std::vector<vec3> object_points = checkerboard_world_corners(cols, rows, square_width);

		real reprojection_error = NAN;
		
		if(color_chk) {
			// calculate extrinsic, and reproject	
			vec3 rotation_vec, translation;
			mat33 rotation;
			cv::solvePnP(
				object_points,
				image_points,
				color_intr.K,
				color_intr.distortion.cv_coeffs(),
				rotation_vec,
				translation,
				false
			);


			std::vector<vec2> reprojected_image_points;
			cv::projectPoints(
				object_points,
				rotation_vec,
				translation,
				color_intr.K,
				color_intr.distortion.cv_coeffs(),
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
								
				cv::circle(color, cv::Point(i_orig[0], i_orig[1]), 10, cv::Scalar(orig_color), 1);
				cv::circle(color, cv::Point(viz_i_reproj[0], viz_i_reproj[1]), 4, cv::Scalar(reproj_color), -1);
			}
			reprojection_error /= rows*cols;
			reprojection_error = std::sqrt(reprojection_error);
		}
		
		view.draw(cv::Rect(0, 0, 1920, 1080), color);
		view.draw_text(cv::Rect(0, 1080, 1920, 30), "rms reprojection error: " + std::to_string(reprojection_error) + " pixel", viewer::center);
		grab.release();

	} while(view.show());
}

