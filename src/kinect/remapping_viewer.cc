#include "../lib/common.h"
#include "../lib/opencv.h"
#include "../lib/json.h"
#include "../lib/obj_img_correspondence.h"
#include "../lib/intrinsics.h"
#include "../lib/misc.h"
#include "../lib/viewer.h"
#include "lib/live/grabber.h"
#include "lib/live/checkerboard.h"
#include "lib/kinect_reprojection.h"
#include <libfreenect2/registration.h>
#include <string>
#include <cmath>

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: remapping_viewer reprojection.json" << std::endl;
	std::exit(1);
}
int main(int argc, const char* argv[]) {
	if(argc <= 1) usage_fail();
	std::string reprojection_parameters_filename = argv[1];

	std::cout << "loading internal&reprojection parameters" << std::endl;
	kinect_reprojection_parameters reprojection_parameters = decode_kinect_reprojection_parameters(import_json_file(reprojection_parameters_filename));

	grabber grab(grabber::color | grabber::ir | grabber::depth);
	libfreenect2::Registration& registration = grab.registration();
	kinect_reprojection	reproj(reprojection_parameters);

	int h = 324;
	int w = 576;
	double scale = 0.3;

	viewer view(3*w, 20+h+30);
	auto& superimpose = view.add_int_slider("superimpose (%)", 0, 0, 100);
	auto& offset = view.add_int_slider("depth offset (-200 + ..) (mm)", 200, -200, 200);
	
	cv::Mat_<uchar> homography_mapping(h, w);
	cv::Mat_<uchar> freenect2_mapping(h, w);
	cv::Mat_<vec2> freenect2_mapping_coordinates(424, 512);
	cv::Mat_<vec2> homography_mapping_coordinates(424, 512);
	cv::Mat_<float> z_buffer(h, w);
		
	do {
		int z_offset = offset.value();

		grab.grab();
		view.clear();
				
		cv::Mat_<cv::Vec3b> color = grab.get_color_frame();
		cv::Mat_<uchar> ir = grab.get_ir_frame();
		cv::Mat_<float> depth = grab.get_depth_frame();

		cv::Mat_<uchar> undistorted_ir = grab.get_ir_frame(true);
		cv::Mat_<float> undistorted_depth = grab.get_depth_frame(true);
		
		// IR+depth to color mapping using Freenect2 registration
		z_buffer.setTo(INFINITY);
		freenect2_mapping.setTo(0);	
		freenect2_mapping_coordinates.setTo(vec2(0,0));
		for(int dy = 0; dy < 424; ++dy) for(int dx = 0; dx < 512; ++dx) {
			float cx, cy;
			float dz = undistorted_depth(dy, dx);
			if(dz < 0.001) continue;			
			uchar value = undistorted_ir(dy, dx);
			registration.apply(dx, dy, dz, cx, cy);
			
			int scx = cx * scale, scy = cy * scale;
			if(scx < 0 || scx >= w || scy < 0 || scy >= h) continue;
			
			float& old_dz = z_buffer(scy, scx);
			if(dz > old_dz) continue;
			freenect2_mapping(scy, scx) = value;
			freenect2_mapping_coordinates(dy, dx) = vec2(cx, cy);
			old_dz = dz;
		}
		

		// IR+depth to color mapping using homography		
		homography_mapping.setTo(0);
		homography_mapping_coordinates.setTo(vec2(0,0));
		z_buffer.setTo(INFINITY);
		
		cv::Mat_<float> depth_off = depth + z_offset;
		depth_off.setTo(0.0, (depth == 0.0));
		auto color_samples = reproj.reproject_ir_to_color_samples<uchar>(ir, depth_off, true);
		for(auto& samp : color_samples) {
			real cx = samp.color_coordinates[0], cy = samp.color_coordinates[1];
			int scx = cx*scale, scy = cy*scale;
			if(scx < 0 || scx >= w || scy < 0 || scy >= h) continue;
			float dz = samp.ir_depth;
			
			float& old_dz = z_buffer(scy, scx);
			if(dz > old_dz) continue;
			homography_mapping(scy, scx) = samp.value;
			int dx = samp.ir_coordinates[0], dy = samp.ir_coordinates[1];
			homography_mapping_coordinates(dy, dx) = samp.color_coordinates;
			old_dz = dz;
		}
	
		// draw images
		view.draw(cv::Rect(0, 20, w, h), freenect2_mapping);
		view.draw(cv::Rect(w, 20, w, h), color);
		view.draw(cv::Rect(2*w, 20, w, h), homography_mapping);
		if(superimpose.value() > 0) {
			float blend = superimpose.value() / 100.0;
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
		view.draw_text(cv::Rect(0, 20+h, 3*w, 30), "rms error: " + std::to_string(err) + " pixel                  z offset: " + std::to_string(z_offset) + " mm", viewer::center);	
	} while(view.show());
}
