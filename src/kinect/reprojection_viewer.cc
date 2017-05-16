#include "../lib/common.h"
#include "../lib/opencv.h"
#include "../lib/json.h"
#include "../lib/obj_img_correspondence.h"
#include "../lib/intrinsics.h"
#include "../lib/utility/misc.h"
#include "lib/live/viewer.h"
#include "lib/live/grabber.h"
#include "lib/live/checkerboard.h"
#include <string>
#include <cmath>

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: reprojection_viewer color_intr.json ir_intr.json rigid.json" << std::endl;
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

	grabber grab(grabber::color | grabber::depth);

	viewer view(754+754, 424);
	auto& min_d = view.add_slider("depth min ", 0, 20000);
	auto& max_d = view.add_slider("depth max", 6000, 20000);
	auto& offset = view.add_slider("depth offset (-500 + ..) (mm)", 500, 1000);
	auto& scaledown = view.add_slider("scaledown (%)", 50, 100);
	auto& superimpose = view.add_slider("superimpose (%)", 0, 100);
	
	cv::Mat_<float> reprojected_depth(1080, 1920);
	cv::Mat_<float> z_buffer(1080, 1920);
	
	do {
		grab.grab();
		view.clear();
		
		cv::Mat_<cv::Vec3b> color = grab.get_color_frame();
		cv::Mat_<float> depth = grab.get_depth_frame();

		float scale = scaledown.value / 100.0;
		int scaled_w = 1920 * scale;
		int scaled_h = 1080 * scale;

		z_buffer.setTo(INFINITY);
		reprojected_depth.setTo(0);
		for(int dy = 0; dy < 424; ++dy) for(int dx = 0; dx < 512; ++dx) {
			float dz = depth(dy, dx);
			if(dz < 0.001) continue;			

			dz += (-500 + offset.value);
			vec3 i_d(dx*dz, dy*dz, dz);
			vec3 v_d = Kinv_ir * i_d;
			vec3 v_c = mul_h(rigid, v_d);
			vec3 i_c = K_color * v_c;
			i_c /= i_c[2];
			
			int cx = scale * i_c[0];
			int cy = scale * i_c[1];
			float cz = v_c[2]; // !!!
			if(cx < 0 || cx >= scaled_w || cy < 0 || cy >= scaled_h) continue;
			
			float& old_cz = z_buffer(cy, cx);
			if(cz > old_cz) continue;
			reprojected_depth(cy, cx) = cz;
			old_cz = cz;
		}
		
		cv::Mat_<float> shown_reprojected_depth;
		cv::resize(cv::Mat(reprojected_depth, cv::Rect(0, 0, scaled_w, scaled_h)), shown_reprojected_depth, cv::Size(1920, 1090), cv::INTER_NEAREST);

		
		view.draw(cv::Rect(0, 0, 754, 424), color);
		view.draw_depth(cv::Rect(754, 0, 754, 424), shown_reprojected_depth, min_d.value, max_d.value);
		if(superimpose.value > 0) {
			float blend = superimpose.value / 100.0;
			view.draw(cv::Rect(754, 0, 754, 424), color, blend);
		}
		
		grab.release();
	} while(view.show());
}
