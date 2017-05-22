#include "../lib/opencv.h"
#include "../lib/intrinsics.h"
#include <cstdlib>
#include <iostream>
#include <string>

using namespace tlz;


intrinsics intr;
cv::Mat_<cv::Vec3b> shown_image;
int cell_width;
int width, height;
std::string window_name;

const cv::Vec3b background_color(0, 0, 0);
const cv::Vec3b label_color(255, 255, 255);
const cv::Vec3b marker_color1(255, 200, 200);
const cv::Vec3b marker_color2(200, 200, 255);
const int max_height = 300;
const int slider_max = 1000;
const real coeff_abs_max = 0.4;
const int border = 20;

int k1_int;
int k2_int;
int k3_int;
int p1_int;
int p2_int;

int reversed_opacity = 0;

void update() {
	// make regular grid
	std::vector<vec2> regular_grid;
	for(int x = width/2; x >= 0; x -= cell_width) {
		for(int y = height/2; y >= 0; y -= cell_width) regular_grid.emplace_back(x, y);
		for(int y = height/2 + cell_width; y < height; y += cell_width) regular_grid.emplace_back(x, y);
	}
	for(int x = width/2 + cell_width; x < width; x += cell_width) {
		for(int y = height/2; y >= 0; y -= cell_width) regular_grid.emplace_back(x, y);
		for(int y = height/2 + cell_width; y < height; y += cell_width) regular_grid.emplace_back(x, y);
	}
	
	// distort + back
	std::vector<vec2> distorted_grid = distort_points(intr, regular_grid);
	std::vector<vec2> undistorted_distorted_grid = undistort_points(intr, distorted_grid);
	
	// undistort + back
	std::vector<vec2> undistorted_grid = undistort_points(intr, regular_grid);
	std::vector<vec2> distorted_undistorted_grid = distort_points(intr, undistorted_grid);
	
	// draw grids
	auto draw_grid = [&](cv::Mat_<cv::Vec3b>& img, const std::vector<vec2>& grid, int x, int y, const cv::Vec3b& col) {
		for(const vec2& pt : grid) {
			int pt_x = pt[0], pt_y = pt[1];
			if(pt_x < 0 || pt_x >= width || pt_y < 0 || pt_y >= height) continue;
			cv::circle(img, cv::Point(x + pt_x, y + pt_y), 3, cv::Scalar(col), 1);			
		}
	};
	cv::Mat_<cv::Vec3b> reversed(height, width);
	real reversed_alpha = reversed_opacity / 100.0;
	shown_image.setTo(background_color);

	// ..draw distorted
	reversed.setTo(background_color);
	draw_grid(reversed, undistorted_distorted_grid, 0, 0, marker_color1);

	draw_grid(shown_image, regular_grid, 0, 20, marker_color1);
	cv::Mat roi = shown_image(cv::Rect(0, 20, width, height));
	cv::addWeighted(reversed, reversed_alpha, roi, 1.0-reversed_alpha, 0.0, roi);

	draw_grid(shown_image, distorted_grid, width+border, 20, marker_color2);

	// ..draw undistorted
	reversed.setTo(background_color);
	draw_grid(reversed, distorted_undistorted_grid, 0, 0, marker_color1);

	draw_grid(shown_image, regular_grid, width+border, height+border+20, marker_color1);
	roi = shown_image(cv::Rect(width+border, height+border+20, width, height));
	cv::addWeighted(reversed, reversed_alpha, roi, 1.0-reversed_alpha, 0.0, roi);

	draw_grid(shown_image, undistorted_grid, 0, height+border+20, marker_color2);
	
	// draw label
	const auto& d = intr.distortion;
	std::string space = "    ";
	std::string label = "k1=" + std::to_string(d.k1) + space + "k2=" + std::to_string(d.k2) + space + "k3=" + std::to_string(d.k3) + space + "p1=" + std::to_string(d.p1) + space + "p2=" + std::to_string(d.p2);
	int font = cv::FONT_HERSHEY_COMPLEX_SMALL;
	double fontscale = 0.7;
	int thickness = 1;
	cv::putText(shown_image, label, cv::Point(10, 2*height+10+20+border), font, fontscale, cv::Scalar(label_color), thickness);
	cv::putText(shown_image, "no distortion", cv::Point(100, 15), font, fontscale, cv::Scalar(label_color), thickness);
	cv::putText(shown_image, "distorted", cv::Point(100+border+width, 15), font, fontscale, cv::Scalar(label_color), thickness);
}



int to_int(real val) {
	return ((val / coeff_abs_max) + 1.0) * slider_max / 2.0;
}

real from_int(int ival) {
	return ((2.0 * ival / slider_max) - 1.0) * coeff_abs_max;
}
 
void update_callback(int = 0, void* = nullptr) {
	if(cell_width <= 1) return;
	
	intr.distortion.k1 = from_int(k1_int);
	intr.distortion.k2 = from_int(k2_int);
	intr.distortion.k3 = from_int(k3_int);
	intr.distortion.p1 = from_int(p1_int);
	intr.distortion.p2 = from_int(p2_int);
	
	update();
	cv::imshow(window_name, shown_image);
}



[[noreturn]] void usage_fail() {
	std::cout << "usage: view_distortion intr.json\n" << std::endl;
	std::exit(EXIT_FAILURE);
}
int main(int argc, const char* argv[]) {
	if(argc <= 1) usage_fail();
	std::string intrinsics_filename = argv[1];

	intr = decode_intrinsics(import_json_file(intrinsics_filename));
	width = intr.width; height = intr.height;
	if(height > max_height) {
		width = max_height * width / height;
		height = max_height;
	}
	
	window_name = "Distortion / Undistortion";
	shown_image = cv::Mat_<cv::Vec3b>(2*height + 20 + 20 + border, 2*width + border);


	k1_int = to_int(intr.distortion.k1);
	k2_int = to_int(intr.distortion.k2);
	k3_int = to_int(intr.distortion.k3);
	p1_int = to_int(intr.distortion.p1);
	p2_int = to_int(intr.distortion.p2);

	cv::namedWindow(window_name, CV_WINDOW_AUTOSIZE);
	cell_width = 20;

	cv::createTrackbar("cell width", window_name, &cell_width, 100, &update_callback);
	cv::createTrackbar("reversed op", window_name, &reversed_opacity, 100, &update_callback);

	cv::createTrackbar("k1", window_name, &k1_int, slider_max, &update_callback);
	cv::createTrackbar("k2", window_name, &k2_int, slider_max, &update_callback);
	cv::createTrackbar("k3", window_name, &k3_int, slider_max, &update_callback);
	cv::createTrackbar("p1", window_name, &p1_int, slider_max, &update_callback);
	cv::createTrackbar("p2", window_name, &p2_int, slider_max, &update_callback);

	update_callback();
	
	cv::waitKey(0);
}
