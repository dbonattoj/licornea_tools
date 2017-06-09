#include "../lib/common.h"
#include "../lib/args.h"
#include "../lib/opencv.h"
#include "../lib/json.h"
#include "../lib/intrinsics.h"
#include "../lib/misc.h"
#include "../lib/viewer.h"
#include "lib/common.h"
#include "lib/live/grabber.h"
#include "lib/live/checkerboard.h"
#include "lib/kinect_reprojection_parameters.h"
#include "lib/kinect_reprojection.h"
#include "lib/densify/depth_densify.h"
#include <string>
#include <cmath>

using namespace tlz;

struct computation_result {
	real hslope;
	real vslope;
	real min_d;
	real max_d;
};
computation_result compute_depth_slopes(const cv::Mat_<real>& depth) {
	std::size_t x_count = depth.cols, y_count = depth.rows;
		
	real min_d = INFINITY;
	real max_d = 0.0;	
	
	std::vector<real> depth_samples;
	depth_samples.reserve(x_count * y_count);
	std::vector<real> hmeans(x_count, 0.0), vmeans(y_count, 0.0);
	std::vector<std::size_t> hcounts(x_count, 0), vcounts(y_count, 0);
	for(std::ptrdiff_t y = 0; y < y_count; ++y)
	for(std::ptrdiff_t x = 0; x < x_count; ++x) {
		real d = depth(y, x);
		if(d == 0.0) continue;
			
		hmeans[x] += d;
		hcounts[x]++;
		vmeans[y] += d;
		vcounts[y]++;
		
		if(d < min_d) min_d = d;
		if(d > max_d) max_d = d;
	}
	
	for(std::ptrdiff_t y = 0; y < y_count; ++y) vmeans[y] /= vcounts[y];
	for(std::ptrdiff_t x = 0; x < x_count; ++x) hmeans[x] /= hcounts[x];
	
	real hslope, vslope;
	{
		std::vector<cv::Vec2f> points; points.reserve(x_count);
		for(std::ptrdiff_t x = 0; x < x_count; ++x)
			if(! std::isnan(hmeans[x])) points.emplace_back(x, hmeans[x]);		
		cv::Vec4f params;
		cv::fitLine(points, params, CV_DIST_L2, 0, 0.01, 0.01);
		hslope = params[1] / params[0];
	}
	
	{
		std::vector<cv::Vec2f> points; points.reserve(y_count);
		for(std::ptrdiff_t y = 0; y < y_count; ++y)
			if(! std::isnan(vmeans[y])) points.emplace_back(y, vmeans[y]);
		cv::Vec4f params;
		cv::fitLine(points, params, CV_DIST_L2, 0, 0.01, 0.01);
		vslope = params[1] / params[0];
	}
		
	return {
		hslope,
		vslope,
		min_d,
		max_d
	};
}


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "reprojection.json");
	std::string reprojection_parameters_filename = in_filename_arg();

	std::cout << "loading reprojection parameters" << std::endl;
	kinect_reprojection_parameters reprojection_parameters = decode_kinect_reprojection_parameters(import_json_file(reprojection_parameters_filename));
	kinect_reprojection reprojection(reprojection_parameters);

	grabber grab(grabber::color | grabber::depth);

	viewer view("Parallel to Wall", 754+754, 424);
	view.indicator_color = cv::Vec3b(0, 0, 255);
	auto& border_x_slider = view.add_int_slider("border X (px)", 280, 250, texture_width/2 - 50);
	auto& border_y_slider = view.add_int_slider("border Y (px)", 100, 0, texture_height/2 - 50);
	auto& indicator_precision_slider = view.add_real_slider("precision", 1.0, 0.1, 2.0);


	auto densifier = make_depth_densify("fast");
	cv::Mat_<real> reprojected_depth;
	cv::Mat_<cv::Vec3b> shown_reprojected_depth;
	
	bool running = true;
	while(running) {
		try {
			grab.grab();
			view.clear();
	
			cv::Mat_<cv::Vec3b> color = grab.get_color_frame();
			cv::Mat_<real> depth = grab.get_depth_frame();
	
			auto samples = reprojection.reproject_ir_to_color_samples(depth, depth, true);
			densifier->densify(samples, reprojected_depth);
	
			grab.release();
	
			// get ROI & compute depth slopes
			int min_x = border_x_slider, max_x = texture_width - border_x_slider;
			int min_y = border_y_slider, max_y = texture_height - border_y_slider;
			
			cv::Rect roi(min_x, min_y, max_x - min_x, max_y - min_y);
			cv::Mat_<real> depth_roi = reprojected_depth(roi);
			
			computation_result res = compute_depth_slopes(depth_roi);
			
			// draw rects
			cv::Mat_<uchar> shown_reprojected_depth = viewer::visualize_depth(reprojected_depth, res.min_d, res.max_d);
			cv::Mat_<cv::Vec3b> shown_reprojected_depth_col;
			cv::cvtColor(shown_reprojected_depth, shown_reprojected_depth_col, CV_GRAY2BGR);
			cv::rectangle(color, roi, cv::Scalar(view.indicator_color), 4);
			cv::rectangle(shown_reprojected_depth_col, roi, cv::Scalar(view.indicator_color), 4);
		
			// draw depth & color
			view.draw(cv::Rect(0, 0, 754, 424), color);
			cv::Rect depth_rect(754, 0, 754, 424);
			view.draw(depth_rect, shown_reprojected_depth_col);
	
			// draw depth indicator
			view.draw_2d_cross_indicator(depth_rect, res.hslope, res.vslope, indicator_precision_slider);
		} catch(...) { }
		
		running = view.show();
	}
}
