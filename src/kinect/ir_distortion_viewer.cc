#include "lib/live/viewer.h"
#include "lib/live/grabber.h"
#include "lib/kinect_internal_parameters.h"
#include "../lib/intrinsics.h"
#include "../lib/json.h"
#include <iostream>
#include <cstdlib>

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: ir_distortion_viewer ir_intrinsics.json/internal" << std::endl;
	std::exit(EXIT_FAILURE);
}
int main(int argc, const char* argv[]) {
	if(argc <= 1) usage_fail();
	std::string ir_intrinsics_filename = argv[1];
	
	grabber grab(grabber::ir);

	viewer view(512+512, 20+424);
	auto& min_ir = view.add_slider("ir min", 0, 0xffff);
	auto& max_ir = view.add_slider("ir max", 0xffff, 0xffff);
	auto& offset = view.add_slider("cell width", 20, 100);
	
	mat33 camera_mat;
	std::vector<real> distortion_coeffs;
	if(ir_intrinsics_filename == "internal") {
		const auto& par = grab.internal_parameters().ir;
		camera_mat = mat33(
			par.fx, 0.0, par.cx,
			0.0, par.fy, par.cy,
			0.0, 0.0, 1.0
		);
		distortion_coeffs = { par.k1, par.k2, par.p1, par.p2, par.k3 };
	} else {
		intrinsics ir_intr = decode_intrinsics(import_json_file(ir_intrinsics_filename));
		camera_mat = ir_intr.K;
		distortion_coeffs = ir_intr.distortion.cv_coeffs();
	}
	
	
	do {
		grab.grab();
		view.clear();
		
		cv::Mat_<cv::Vec3b> raw_ir, undistorted_ir;
		cv::cvtColor(grab.get_ir_frame(min_ir.value, max_ir.value, false), raw_ir, CV_GRAY2BGR);
		grab.release();
		
		cv::undistort(raw_ir, undistorted_ir, camera_mat, distortion_coeffs, camera_mat);
		
		if(offset.value > 3) {
			std::vector<cv::Point2f> raw_pts, undistorted_pts;
			for(int x = 512/2; x >= 0; x -= offset.value) {
				for(int y = 424/2; y >= 0; y -= offset.value) raw_pts.emplace_back(x, y);
				for(int y = 424/2; y < 424; y += offset.value) raw_pts.emplace_back(x, y);			
			}
			for(int x = 512/2; x < 512; x += offset.value) {
				for(int y = 424/2; y >= 0; y -= offset.value) raw_pts.emplace_back(x, y);
				for(int y = 424/2; y < 424; y += offset.value) raw_pts.emplace_back(x, y);			
			}
			
			cv::undistortPoints(raw_pts, undistorted_pts, camera_mat, distortion_coeffs, cv::noArray(), camera_mat);
			
			cv::Vec3b raw_point_color(255, 200, 200);
			cv::Vec3b undistorted_point_color(200, 200, 255);
			for(const cv::Point2f& pt : raw_pts) cv::circle(raw_ir, pt, 4, cv::Scalar(raw_point_color), 2);
			for(const cv::Point2f& pt : undistorted_pts) cv::circle(undistorted_ir, pt, 4, cv::Scalar(undistorted_point_color), 2);
		}
				
		view.draw_text(cv::Rect(0, 0, 512, 20), "raw", viewer::center);
		view.draw(cv::Rect(0, 20, 512, 424), raw_ir);

		view.draw_text(cv::Rect(512, 0, 512, 20), "undistorted", viewer::center);
		view.draw(cv::Rect(512, 20, 512, 424), undistorted_ir);
	} while(view.show());	
}
