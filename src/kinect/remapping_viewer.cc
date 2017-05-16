#include "../lib/common.h"
#include "../lib/opencv.h"
#include "../lib/json.h"
#include "../lib/obj_img_correspondence.h"
#include "../lib/intrinsics.h"
#include "../lib/utility/misc.h"
#include "lib/live/viewer.h"
#include "lib/live/grabber.h"
#include "lib/live/checkerboard.h"
#include <libfreenect2/registration.h>
#include <string>
#include <cmath>

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: remapping_viewer color_intr.json ir_intr.json rigid.json" << std::endl;
	std::exit(1);
}
int main(int argc, const char* argv[]) {
	if(argc <= 3) usage_fail();
	std::string color_intrinsics_filename = argv[1];
	std::string ir_intrinsics_filename = argv[2];
	std::string rigid_filename = argv[3];

	std::cout << "loading intrinsics and rigid" << std::endl;
	intrinsics color_intr = decode_intrinsics(import_json_file(color_intrinsics_filename));
	intrinsics ir_intr = decode_intrinsics(import_json_file(ir_intrinsics_filename));
	mat44 rigid = decode_mat(import_json_file(rigid_filename));
	rigid = rigid.inv();

	mat33 K_color = color_intr.K;
	mat33 Kinv_ir = ir_intr.K.inv();

	grabber grab(grabber::color | grabber::ir | grabber::depth);
	libfreenect2::Registration& registration = grab.registration();

	int h = 324;
	int w = 576;
	double scale = 0.3;

	viewer view(3*w, 20+h+30);
	auto& superimpose = view.add_slider("superimpose (%)", 0, 100);
	auto& offset = view.add_slider("depth offset (-500 + ..) (mm)", 500, 1000);
	
	cv::Mat_<uchar> homography_mapping(h, w);
	cv::Mat_<uchar> freenect2_mapping(h, w);
	cv::Mat_<vec2> freenect2_mapping_coordinates(424, 512);
	cv::Mat_<vec2> homography_mapping_coordinates(424, 512);
	cv::Mat_<float> z_buffer(h, w);
	
	do {
		grab.grab();
		view.clear();
				
		cv::Mat_<cv::Vec3b> color = grab.get_color_frame();
		cv::Mat_<uchar> ir = grab.get_ir_frame();
		cv::Mat_<float> depth = grab.get_depth_frame();


		// IR+depth to color mapping using Freenect2 registration
		z_buffer.setTo(INFINITY);
		freenect2_mapping.setTo(0);	
		freenect2_mapping_coordinates.setTo(vec2(0,0));
		for(int dy = 0; dy < 424; ++dy) for(int dx = 0; dx < 512; ++dx) {
			float cx, cy;
			float dz = depth(dy, dx);
			if(dz < 0.001) continue;			
			uchar value = ir(dy, dx);
			registration.apply(dx, dy, dz, cx, cy);
			cx *= scale;
			cy *= scale;
			if(cx < 0 || cx >= w || cy < 0 || cy >= h) continue;
			
			float& old_dz = z_buffer(cy, cx);
			if(dz > old_dz) continue;
			freenect2_mapping(cy, cx) = value;
			freenect2_mapping_coordinates(dy, dx) = vec2(cy, cx);
			old_dz = dz;
		}
	
		// IR+depth to color mapping using homography
		z_buffer.setTo(INFINITY);
		homography_mapping.setTo(0);
		homography_mapping_coordinates.setTo(vec2(0,0));
		for(int dy = 0; dy < 424; ++dy) for(int dx = 0; dx < 512; ++dx) {
			float dz = depth(dy, dx);
			if(dz < 0.001) continue;			
			uchar value = ir(dy, dx);

			dz += (-500 + offset.value);
			vec3 i_d(dx*dz, dy*dz, dz);
			vec3 v_d = Kinv_ir * i_d;
			vec3 v_c = mul_h(rigid, v_d);
			vec3 i_c = K_color * v_c;
			i_c /= i_c[2];
			
			int cx = i_c[0] * scale;
			int cy = i_c[1] * scale;
			if(cx < 0 || cx >= w || cy < 0 || cy >= h) continue;
			
			float& old_dz = z_buffer(cy, cx);
			if(dz > old_dz) continue;
			homography_mapping(cy, cx) = value;
			homography_mapping_coordinates(dy, dx) = vec2(cy, cx);
			old_dz = dz;
		}

		// draw images
		view.draw(cv::Rect(0, 20, w, h), freenect2_mapping);
		view.draw(cv::Rect(w, 20, w, h), color);
		view.draw(cv::Rect(2*w, 20, w, h), homography_mapping);
		if(superimpose.value > 0) {
			float blend = superimpose.value / 100.0;
			view.draw(cv::Rect(0, 20, w, h), color, blend);
			view.draw(cv::Rect(2*w, 20, w, h), color, blend);
		}

		grab.release();
	

		// measure error
		real sum = 0.0;
		int count = 0;
		for(int dy = 0; dy < 424; ++dy) for(int dx = 0; dx < 512; ++dx) {
			vec2 freenect2_mapping_coord = freenect2_mapping_coordinates(dy, dx);
			vec2 homography_mapping_coord = homography_mapping_coordinates(dy, dx);
			if(freenect2_mapping_coord == vec2(0,0) || homography_mapping_coord == vec2(0,0)) continue;
			
			vec2 diff = freenect2_mapping_coord - homography_mapping_coord;
			sum += sq(diff[0]) + sq(diff[1]);
			count++;
		}
		real err = std::sqrt(sum / count);


		view.draw_text(cv::Rect(0, 0, w, 20), "Freenect2 Registration", viewer::center);
		view.draw_text(cv::Rect(2*w, 0, w, 20), "Homography", viewer::center);	
		view.draw_text(cv::Rect(0, 20+h, 3*w, 30), "rms error: " + std::to_string(err) + " pixel                  z offset: " + std::to_string(-500 + offset.value) + " mm", viewer::center);	
	} while(view.show());
}
