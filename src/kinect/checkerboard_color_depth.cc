int main() {}
/*
#include "../lib/common.h"
#include "../lib/opencv.h"
#include "../lib/json.h"
#include "../lib/obj_img_correspondence.h"
#include "../lib/intrinsics.h"
#include "../lib/utility/misc.h"
#include "lib/live/viewer.h"
#include "lib/live/grabber.h"
#include "lib/live/checkerboard.h"
#include "lib/kinect_reprojection_parameters.h"
#include <string>
#include <cmath>

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: checkerboard_color_depth cols rows square_width reprojection.json" << std::endl;
	std::exit(1);
}
int main(int argc, const char* argv[]) {
	if(argc <= 4) usage_fail();
	int cols = std::atoi(argv[1]);
	int rows = std::atoi(argv[2]);
	real square_width = std::atof(argv[3]);
	std::string reprojection_parameters_filename = argv[4];

	std::cout << "loading reprojection parameters" << std::endl;
	kinect_reprojection_parameters reprojection_parameters = decode_kinect_reprojection_parameters(import_json_file(reprojection_parameters_filename));

	grabber grab(grabber::color | grabber::depth);

	viewer view(754+754, 424+30+30);
	auto& min_d = view.add_slider("depth min ", 0, 20000);
	auto& max_d = view.add_slider("depth max", 6000, 20000);
	auto& offset = view.add_slider("depth offset (-200 + ..) (mm)", 200, 400);
	auto& scaledown = view.add_slider("scaledown (%)", 30, 100);
	auto& superimpose = view.add_slider("superimpose (%)", 0, 100);
		
	cv::Mat_<float> reprojected_depth(1080, 1920);
	cv::Mat_<float> z_buffer(1080, 1920);
	
	bool running = true;
	while(running) {
		int z_offset = -200 + offset.value;
		
		grab.grab();
		view.clear();
		
		cv::Mat_<cv::Vec3b> color = grab.get_color_frame();
		cv::Mat_<float> depth = grab.get_depth_frame();

		// make reprojected depth
		float depth_scale = scaledown.value / 100.0;
		int depth_scaled_w = 1920 * depth_scale;
		int depth_scaled_h = 1080 * depth_scale;
		z_buffer.setTo(INFINITY);
		reprojected_depth.setTo(0);
		for(int dy = 0; dy < 424; ++dy) for(int dx = 0; dx < 512; ++dx) {
			float dz = depth(dy, dx);
			if(dz < 0.001) continue;			

			dz += z_offset;
			vec3 i_d(dx*dz, dy*dz, dz);
			vec3 v_d = reprojection_parameters.ir_intrinsic_inv * i_d;
			vec3 v_c = reprojection_parameters.rotation.t() * (v_d - reprojection_parameters.translation);
			vec3 i_c = reprojection_parameters.color_intrinsic * v_c;
			i_c /= i_c[2];
			
			int cx = depth_scale * i_c[0];
			int cy = depth_scale * i_c[1];
			float cz = v_c[2];
			if(cx < 0 || cx >= depth_scaled_w || cy < 0 || cy >= depth_scaled_h) continue;
			
			float z = cz;
			
			float& old_cz = z_buffer(cy, cx);
			if(cz > old_cz) continue;
			reprojected_depth(cy, cx) = z;
			old_cz = cz;
		}
		cv::Mat_<float> shown_reprojected_depth;
		cv::resize(cv::Mat(reprojected_depth, cv::Rect(0, 0, depth_scaled_w, depth_scaled_h)), shown_reprojected_depth, cv::Size(1920, 1090), cv::INTER_NEAREST);
		
		// detect checkerboard in color image
		checkerboard color_chk = detect_color_checkerboard(color, cols, rows, square_width);
		
		std::vector<real> projection_depths, reprojected_measured_depths;
		real avg_projection_depth = NAN, avg_reprojected_measured_depth = NAN;
		real avg_error = NAN, stddev_error = NAN;
		int count = 0;
		if(color_chk) {
			std::vector<vec2> image_points = checkerboard_image_corners(color_chk);
			std::vector<vec3> object_points = checkerboard_world_corners(cols, rows, square_width);

			// calculate extrinsic	
			vec3 rotation_vec, translation;
			mat33 rotation;
			cv::solvePnP(
				object_points,
				image_points,
				reprojection_parameters.color_intrinsics.K,
				reprojection_parameters.color_intrinsics.distortion.cv_coeffs(),
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
				reprojected_measured_depths.push_back(0.0);
				real& reprojected_measured_d = reprojected_measured_depths.back();
				
				int sdx = depth_scale * i[0], sdy = depth_scale * i[1];
				if(sdx < 0 || sdx >= depth_scaled_w || sdy < 0 || sdy >= depth_scaled_h) continue;
				
				float dz = reprojected_depth(sdy, sdx);
				if(dz < 0.001) continue;
				
				reprojected_measured_d = dz;
			}
			
			// calculate avg and stddev error
			std::vector<real> errs;
			assert(projection_depths.size() == reprojected_measured_depths.size());
			count = projection_depths.size();
			avg_error = 0.0; stddev_error = 0.0;
			avg_projection_depth = 0.0; avg_reprojected_measured_depth = 0.0;
			for(int i = 0; i < projection_depths.size(); ++i) {
				if(reprojected_measured_depths[i] == 0.0 || projection_depths[i] == 0.0) {
					--count;
					continue;
				}
				
				real err = reprojected_measured_depths[i] - projection_depths[i];
				errs.push_back(err);
				avg_error += err;
				
				avg_projection_depth += projection_depths[i];
				avg_reprojected_measured_depth += reprojected_measured_depths[i];
			}
			avg_error /= count;
			for(real err : errs) stddev_error += sq(err - avg_error);
			stddev_error /= count;
			stddev_error = std::sqrt(stddev_error);
			avg_projection_depth /= count;
			avg_reprojected_measured_depth /= count;
		}
		
		cv::Mat_<cv::Vec3b> shown_color = visualize_checkerboard(color, color_chk);
		view.draw(cv::Rect(0, 0, 754, 424), shown_color);
		cv::Rect depth_rect(754, 0, 754, 424);
		view.draw_depth(depth_rect, shown_reprojected_depth, min_d.value, max_d.value);
		if(superimpose.value > 0) {
			float blend = superimpose.value / 100.0;
			view.draw(depth_rect, shown_color, blend);
		}
		
		view.draw_text(cv::Rect(20, 424, 754+754, 30), "avg projection depth: " + std::to_string(avg_projection_depth) + " mm", viewer::left);
		view.draw_text(cv::Rect(20+500, 424, 754+754, 30), "avg reprojected measured depth: " + std::to_string(avg_reprojected_measured_depth) + " mm", viewer::left);
		view.draw_text(cv::Rect(20+1100, 424, 754+754, 30), "depth offset: " + std::to_string(z_offset) + " mm", viewer::left);


		view.draw_text(cv::Rect(20, 424+30, 754+754, 30), "error avg: " + std::to_string(avg_error) + " mm", viewer::left);
		view.draw_text(cv::Rect(20+300, 424+30, 754+754, 30), "stddev: " + std::to_string(stddev_error) + " mm", viewer::left);
		view.draw_text(cv::Rect(20+600, 424+30, 754+754, 30), "samples count: " + std::to_string(count), viewer::left);
		
		grab.release();
		
		running = view.show();
	};
}
*/
