#include "../lib/args.h"
#include "../lib/opencv.h"
#include "../lib/viewer.h"
#include "../lib/intrinsics.h"
#include <cstdlib>
#include <iostream>
#include <string>

using namespace tlz;

const cv::Vec3b marker_color1(255, 200, 200);
const cv::Vec3b marker_color2(200, 200, 255);

const int max_height = 300;
const int slider_steps = 1000;
const real coeff_abs_max = 0.4;
const int border = 20;


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "intr.json");
	intrinsics intr = intrinsics_arg();

	int width = intr.width;
	int height = intr.height;
	real scale = 1.0;
	if(height > max_height) {
		scale = real(height) / max_height;		
		width = max_height * width / height;
		height = max_height;
	}
	
	viewer view("Distortion / Undistortion", 2*width + border, 2*height + 20 + 20 + border);
	auto& k1_slider = view.add_real_slider("k1", intr.distortion.k1, -coeff_abs_max, +coeff_abs_max, slider_steps);
	auto& k2_slider = view.add_real_slider("k2", intr.distortion.k2, -coeff_abs_max, +coeff_abs_max, slider_steps);
	auto& k3_slider = view.add_real_slider("k3", intr.distortion.k3, -coeff_abs_max, +coeff_abs_max, slider_steps);
	auto& p1_slider = view.add_real_slider("p1", intr.distortion.p1, -coeff_abs_max, +coeff_abs_max, slider_steps);
	auto& p2_slider = view.add_real_slider("p2", intr.distortion.p2, -coeff_abs_max, +coeff_abs_max, slider_steps);
	auto& cell_width_slider = view.add_int_slider("cell width", 20, 3, 100);
	auto& reversed_opacity_slider = view.add_int_slider("reversed op", 0, 0, 100);
		
	view.update_callback = [&]() {
		intr.distortion.k1 = k1_slider.value();
		intr.distortion.k2 = k2_slider.value();
		intr.distortion.k3 = k3_slider.value();
		intr.distortion.p1 = p1_slider.value();
		intr.distortion.p2 = p2_slider.value();
		int cell_width = cell_width_slider.value();
		int reversed_opacity = reversed_opacity_slider.value();
		
		// make regular grid
		std::vector<vec2> small_regular_grid;
		for(int x = width/2; x >= 0; x -= cell_width) {
			for(int y = height/2; y >= 0; y -= cell_width) small_regular_grid.emplace_back(x, y);
			for(int y = height/2 + cell_width; y < height; y += cell_width) small_regular_grid.emplace_back(x, y);
		}
		for(int x = width/2 + cell_width; x < width; x += cell_width) {
			for(int y = height/2; y >= 0; y -= cell_width) small_regular_grid.emplace_back(x, y);
			for(int y = height/2 + cell_width; y < height; y += cell_width) small_regular_grid.emplace_back(x, y);
		}
		std::vector<vec2> regular_grid;
		for(const vec2& pt : small_regular_grid) regular_grid.push_back(scale * pt);
	
		
		// distort + back
		std::vector<vec2> distorted_grid = distort_points(intr, regular_grid);
		std::vector<vec2> undistorted_distorted_grid = undistort_points(intr, distorted_grid);
		
		// undistort + back
		std::vector<vec2> undistorted_grid = undistort_points(intr, regular_grid);
		std::vector<vec2> distorted_undistorted_grid = distort_points(intr, undistorted_grid);
		
		// draw grids
		auto drawn_grid = [&](const std::vector<vec2>& grid, const cv::Vec3b& col) {
			cv::Mat_<cv::Vec3b> img(height, width);
			img.setTo(view.background_color);
			for(const vec2& pt : grid) {
				int pt_x = pt[0] / scale, pt_y = pt[1] / scale;
				if(pt_x < 0 || pt_x >= width || pt_y < 0 || pt_y >= height) continue;
				cv::circle(img, cv::Point(pt_x, pt_y), 3, cv::Scalar(col), 1);			
			}
			return img;
		};
		cv::Mat_<cv::Vec3b> reversed(height, width);
		real reversed_alpha = reversed_opacity / 100.0;
	
		view.clear();

		// ..upper row (regular & undistorted_distorted  |  distorted)
		view.draw(cv::Point(0, 20), drawn_grid(regular_grid, marker_color1));
		view.draw(cv::Point(0, 20), drawn_grid(undistorted_distorted_grid, marker_color1), reversed_alpha);
		view.draw(cv::Point(width+border, 20), drawn_grid(distorted_grid, marker_color2));
		
		// ..lower row (undistorted  |  regular & distorted_undistorted)
		view.draw(cv::Point(0, height+border+20), drawn_grid(undistorted_grid, marker_color2));
		view.draw(cv::Point(width+border, height+border+20), drawn_grid(regular_grid, marker_color1));
		view.draw(cv::Point(width+border, height+border+20), drawn_grid(distorted_undistorted_grid, marker_color1), reversed_alpha);

		
		// draw label
		std::string space = "    ";
		const auto& d = intr.distortion;
		std::string label = "k1=" + std::to_string(d.k1) + space + "k2=" + std::to_string(d.k2) + space + "k3=" + std::to_string(d.k3) + space + "p1=" + std::to_string(d.p1) + space + "p2=" + std::to_string(d.p2);
		view.draw_text(cv::Rect(10, 2*height+20+border, 2*width+border-20, 20), label, viewer::center);
		view.draw_text(cv::Rect(0, 0, width, 20), "no distortion");
		view.draw_text(cv::Rect(width+border, 0, width, 20), "distorted");
	};
	
	view.show_modal();
}
