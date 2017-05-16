#include "../lib/common.h"
#include "../lib/intrinsics.h"
#include "lib/live/viewer.h"
#include "lib/live/grabber.h"
#include "lib/live/checkerboard.h"

using namespace tlz;


[[noreturn]] void usage_fail() {
	std::cout << "usage: live_ir_depth_chk_distance cols rows square_width\n";
	std::cout << std::endl;
	std::exit(1);
}
int main(int argc, const char* argv[]) {
	if(argc <= 4) usage_fail();
	const int cols = std::stoi(argv[1]);
	const int rows = std::stoi(argv[2]);
	const real square_width = std::stof(argv[3]);
	
	grabber grab(grabber::depth | grabber::ir);

	viewer view(512+512, 424);
	auto& min_d = view.add_slider("depth min ", 0, 20000);
	auto& max_d = view.add_slider("depth max", 6000, 20000);
	auto& min_ir = view.add_slider("ir min", 0, 0xffff);
	auto& max_ir = view.add_slider("ir max", 0xffff, 0xffff);
	
	cv::Mat_<uchar> chk_depth_mask(424, 512);
	
	for(;;) {
		grab.grab();
		view.clear();
		
		cv::Mat_<uchar> ir = grab.get_ir_frame(min_ir.value, max_ir.value);
		cv::Mat_<float> depth = grab.get_depth_frame();
		
		// detect checkerboard in IR image
		checkerboard chk = detect_checkerboard(ir, cols, rows);

		if(chk) {
			// get IR sensor intrinsic
			auto ir_param = grab.device().getIrCameraParams();
			mat33 intrinsic(
				365.456f, 0.0, 254.878f,
				0.0, 365.456f, 205.395f,
				0.0, 0.0, 1.0
			);
			
			// distance of checkerboard using intrinsic
			std::vector<vec3> world_points = checkerboard_world_corners(cols, rows, square_width);
			std::vector<vec2> image_points = checkerboard_image_corners(chk);
			
			mat33 rotation;
			vec3 rotation_vec, translation;
			cv::Mat distortion;
								
			cv::solvePnP(world_points, image_points, intrinsic, distortion, rotation_vec, translation);
								
			mat33 rotation_mat;
			cv::Rodrigues(rotation_vec, rotation);
			
			std::cout << rotation_vec[0] << ", " << rotation_vec[1] << ", " << rotation_vec[2] << std::endl;
			
			std::cout << "proj: " << translation[2] << std::endl;
			
			// same in depth
			chk_depth_mask.setTo(0);
			auto outer_corners_i = chk.outer_corners_i();
			cv::fillConvexPoly(chk_depth_mask, outer_corners_i.data(), 4, cv::Scalar(255));
			chk_depth_mask.setTo(0, (depth < 0.001));
			
			real depth_sum = 0.0;
			int depth_count = 0;
			for(int y = 0; y <= chk.bounding_rect.height; ++y)
			for(int x = 0; x <= chk.bounding_rect.width; ++x) {
				int gx = chk.bounding_rect.x + x;
				int gy = chk.bounding_rect.y + y;
				
				uchar mask = chk_depth_mask(gy, gx);
				if(! mask) continue;
				real d = depth(gy, gx);
				depth_sum += d;
				depth_count++;
			}
			real sensor_depth = depth_sum / depth_count;

			std::cout << "sensor: " << sensor_depth << std::endl;
		}
		
		
		view.draw(cv::Rect(0, 0, 512, 424), ir);
		view.draw_depth(cv::Rect(512, 0, 512, 424), depth, min_d.value, max_d.value);

		if(chk) {
			cv::Mat_<cv::Vec3b> ir_viz = visualize_checkerboard(ir, chk);
			view.draw(cv::Rect(0, 0, 512, 424), ir_viz);
		}
	
		grab.release();
		view.show();
	}
}
