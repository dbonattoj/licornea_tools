#include <json.hpp>
#include "lib/common.h"
#include "../lib/opencv.h"
#include "lib/freenect2.h"
#include "lib/kinect_intrinsics.h"
#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/registration.h>
#include <libfreenect2/packet_pipeline.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <memory>
#include <cassert>
#include <cmath>

using namespace tlz;

const cv::Vec3b black(0, 0, 0), white(255, 255, 255);

real stddev(const std::vector<real>& samples, real* avg_ptr = nullptr) {
	real avg = 0.0;
	for(real sample : samples) avg += sample;
	avg /= samples.size();
	real stddev = 0.0;
	for(real sample : samples) stddev += (avg - sample)*(avg - sample);
	stddev /= samples.size();
	if(avg_ptr) *avg_ptr = avg;
	return std::sqrt(stddev);
}

cv::Mat_<uchar> scale_grayscale(cv::Mat in, real min, real max) {
	cv::Mat_<uchar> scaled;
	float alpha = 255.0f / (max - min);
	float beta = -alpha * min;
	cv::convertScaleAbs(in, scaled, alpha, beta);
	scaled.setTo(0, (in < min));
	scaled.setTo(255, (in > max));
	scaled.setTo(0, (in == 0));
	return scaled;
}

void draw_indicator(cv::Mat_<cv::Vec3b>& image, cv::Rect_<int> rect, real value, real max, cv::Vec3b col) {
	cv::rectangle(image, rect, cv::Scalar(black), CV_FILLED);
	cv::Rect_<int> value_rect = rect;
	value_rect.width *= value/max;
	if(value_rect.width > rect.width) value_rect.width = rect.width;
	cv::rectangle(image, value_rect, cv::Scalar(col), CV_FILLED);
	cv::rectangle(image, rect, cv::Scalar(white), 1);
}


void draw_dots(cv::Mat_<cv::Vec3b>& image, cv::vector<cv::Point2f>& dots, cv::Vec3b col) {
	for(const cv::Point2f& dot : dots)
		cv::circle(image, dot, 5, cv::Scalar(col), CV_FILLED);
}


std::vector<cv::Point> checkerboard_outer_corners(int cols, int rows, const std::vector<cv::Point2f>& corners) {
	return {
		corners.at(0),
		corners.at(cols-1),
		corners.at(cols*(rows-1) + (cols-1)),
		corners.at(cols*(rows-1))
	};
}


void draw_detected_checkerboard(cv::Mat_<cv::Vec3b>& image, int cols, int rows, real visible_line_outreach, const std::vector<cv::Point2f>& corners, const cv::Vec3b& col) {
	std::vector<std::vector<cv::Point>> polylines;
	
	real r = visible_line_outreach;
	
	auto outer_corners = checkerboard_outer_corners(cols, rows, corners);
	cv::Point2f tl = outer_corners[0];
	cv::Point2f tr = outer_corners[1];
	cv::Point2f br = outer_corners[2];
	cv::Point2f bl = outer_corners[3];
	
	real hslope1 = (tr.y - tl.y) / (tr.x - tl.x);
	real hslope2 = (br.y - bl.y) / (br.x - bl.x);
	{
		std::vector<cv::Point> segment;
		segment.emplace_back(tl.x - r, tl.y - hslope1*r);
		segment.emplace_back(tr.x + r, tr.y + hslope1*r);
		polylines.push_back(segment);
	}
	{
		std::vector<cv::Point> segment;
		segment.emplace_back(bl.x - r, bl.y - hslope2*r);
		segment.emplace_back(br.x + r, br.y + hslope2*r);
		polylines.push_back(segment);
	}
	
	real vslope1 = (tl.x - bl.x) / (tl.y - bl.y);
	real vslope2 = (tr.x - br.x) / (tr.y - br.y);
	{
		std::vector<cv::Point> segment;
		segment.emplace_back(tl.x - vslope1*r, tl.y - r);
		segment.emplace_back(bl.x + vslope1*r, bl.y + r);
		polylines.push_back(segment);
	}
	{
		std::vector<cv::Point> segment;
		segment.emplace_back(tr.x - vslope2*r, tr.y - r);
		segment.emplace_back(br.x + vslope2*r, br.y + r);
		polylines.push_back(segment);
	}
	
				
	cv::polylines(image, polylines, false, cv::Scalar(col), 4);
}



int main(int argc, const char* argv[]) {
	using namespace libfreenect2;

	Freenect2 context;
	int count = context.enumerateDevices();
	if(count == 0) throw std::runtime_error("Kinect not found");
		
	CpuPacketPipeline pipeline;
	
	std::string serial = context.getDefaultDeviceSerialNumber();
	std::unique_ptr<Freenect2Device> device(context.openDevice(serial, &pipeline));
	if(device == nullptr) throw std::runtime_error("could not open device");
	
	int types = Frame::Color | Frame::Depth;
	SyncMultiFrameListener listener(types);
	FrameMap frames;
	device->setColorFrameListener(&listener);
	device->setIrAndDepthFrameListener(&listener);

	bool ok = device->start();
	if(! ok) throw std::runtime_error("could not start device");

	Freenect2Device::ColorCameraParams color = device->getColorCameraParams();
	Freenect2Device::IrCameraParams ir = device->getIrCameraParams();

	std::string window_name = "Viewer";
	cv::namedWindow(window_name, CV_WINDOW_AUTOSIZE);
	int max_depth = 6000; // mm
	int z_near = 0, z_far = max_depth;
	int z_checkerboard_range = 150;
	cv::createTrackbar("min depth (mm)", window_name, &z_near, max_depth);
	cv::createTrackbar("max depth (mm)", window_name, &z_far, max_depth);
	cv::createTrackbar("range of checkerboard depth (mm)", window_name, &z_checkerboard_range, 1000);

	Frame undistorted_depth(512, 424, 4);
	Frame registered_texture(512, 424, 4);
	Frame upscaled_depth(1920, 1082, 4);
	Registration registration(ir, color);
	
	const int indicator_height = 30;
	const int indicator_border = 5;
	const cv::Vec3b slope_indicator_color = cv::Vec3b(255,100,100);
	const cv::Vec3b depth_indicator_color = cv::Vec3b(100,100,255);
	
	cv::Mat_<cv::Vec3b> shown_img(400 + indicator_height + 2*indicator_border, 711*2);
	cv::Mat_<float> depth_frame(1080, 1920);
	cv::Mat_<cv::Vec3b> texture_frame(1080, 1920);
	cv::Mat_<uchar> checkerboard_depth_mask(1080, 1920);

	bool continuing = true;
	while(continuing) {
		ok = listener.waitForNewFrame(frames, 10*1000);
		if(! ok) break;

		Frame* raw_texture = frames[Frame::Color];
		Frame* raw_depth = frames[Frame::Depth];
		registration.apply(raw_texture, raw_depth, &undistorted_depth, &registered_texture, true, &upscaled_depth);
				
		{
			cv::Mat_<float> depth_orig_float(1082, 1920, reinterpret_cast<float*>(upscaled_depth.data));		
			cv::Mat_<float> depth_float = depth_orig_float.rowRange(1, 1081);
			depth_frame = depth_float;
			depth_frame.setTo(0, depth_float == INFINITY);
		}

		{
			cv::Mat_<cv::Vec4b> texture_orig(1080, 1920, reinterpret_cast<cv::Vec4b*>(raw_texture->data));
			cv::cvtColor(texture_orig, texture_frame, CV_BGRA2BGR);
		}
		
		listener.release(frames);

		const int cols = 8, rows = 6;
		real visible_line_outreach = 400;
		
		std::vector<cv::Point2f> corners;
		cv::Size pattern_size(cols, rows);
	
		int flags = 0;
		bool found_checkerboard = cv::findChessboardCorners(texture_frame, pattern_size, corners, flags);
		if(corners.size() != rows*cols) found_checkerboard = false;
	
		const real slope_planarity_value_max = 0.15;
		real slope_planarity_value = slope_planarity_value_max;
		
		const real depth_planarity_value_max = 50.0;
		real depth_planarity_value = depth_planarity_value_max;
		cv::vector<cv::Point2f> measured_depth_corners;
		real min_d = z_near, max_d = z_far;
		cv::Rect checkerboard_rect(0, 0, 1920, 1080);
		real avg_depth = NAN;
	
		checkerboard_depth_mask.setTo(0);
		if(found_checkerboard) {
			draw_detected_checkerboard(texture_frame, cols, rows, visible_line_outreach, corners, slope_indicator_color);
			draw_dots(texture_frame, corners, slope_indicator_color);
			
			std::vector<real> hslopes;
			for(int row = 0; row < rows; ++row) {
				cv::Point2f l = corners.at(cols*row);
				cv::Point2f r = corners.at(cols*row + (cols-1));
				hslopes.push_back( (r.y - l.y) / (r.x - l.x) );
			}
			
			std::vector<real> vslopes;
			for(int col = 0; col < cols; ++col) {
				cv::Point2f t = corners.at(col);
				cv::Point2f b = corners.at(cols*(rows-1) + col);
				vslopes.push_back( (b.x - t.x) / (b.y - t.y) );
			}
			
			real h_stddev = stddev(hslopes);
			real v_stddev = stddev(vslopes);
			slope_planarity_value = h_stddev + v_stddev;
			
				
			std::vector<cv::Point> outer_corners = checkerboard_outer_corners(cols, rows, corners);
			cv::fillConvexPoly(checkerboard_depth_mask, outer_corners.data(), 4, cv::Scalar(255));
			checkerboard_depth_mask.setTo(0, (depth_frame == 0.0));

			int min_x = 1920, max_x = 0, min_y = 1080, max_y = 0;
			for(const cv::Point& corner : outer_corners) {
				if(corner.x < min_x) min_x = corner.x;
				if(corner.x > max_x) max_x = corner.x;
				if(corner.y < min_y) min_y = corner.y;
				if(corner.y > max_y) max_y = corner.y;				
			}
			checkerboard_rect.x = min_x;
			checkerboard_rect.y = min_y;
			checkerboard_rect.width = max_x - min_x;
			checkerboard_rect.height = max_y - min_y;
			
			cv::vector<real> depth_samples;
			min_d = INFINITY; max_d = 0.0;
			for(int y = min_y; y <= max_y; ++y) for(int x = min_x; x <= max_x; ++x) {
				uchar mask = checkerboard_depth_mask(y, x);
				if(! mask) continue;
				real d = depth_frame(y, x);
				depth_samples.push_back(d);
				if(d < min_d) min_d = d;
				if(d > max_d) max_d = d;
			}
			real mid_d = (max_d + min_d)/2.0;
			min_d = mid_d - z_checkerboard_range/2.0;
			max_d = mid_d + z_checkerboard_range/2.0;
			depth_planarity_value = stddev(depth_samples, &avg_depth);
		}



	
		{
			cv::Mat_<cv::Vec3b> texture, depth;
			cv::resize(texture_frame, texture, cv::Size(711, 400));
			
			cv::Mat_<uchar> depth_uchar = scale_grayscale(depth_frame, min_d, max_d);
			cv::cvtColor(depth_uchar, depth, CV_GRAY2BGR);
			
			if(found_checkerboard) depth.setTo(cv::Vec3b(0,0,0), (checkerboard_depth_mask == 0));
			cv::resize(depth, depth, cv::Size(711, 400));
			
			texture.copyTo(cv::Mat(shown_img, cv::Rect(0, 0, 711, 400)));
			depth.copyTo(cv::Mat(shown_img, cv::Rect(711, 0, 711, 400)));
			
			cv::Rect_<int> slope_indicator(indicator_border, 400+indicator_border, 711-2*indicator_border, indicator_height);
			draw_indicator(shown_img, slope_indicator, slope_planarity_value, slope_planarity_value_max, slope_indicator_color);

			cv::Rect_<int> depth_indicator(711+indicator_border, 400+indicator_border, 711-2*indicator_border, indicator_height);
			draw_indicator(shown_img, depth_indicator, depth_planarity_value, depth_planarity_value_max, depth_indicator_color);

			if(found_checkerboard) {
				std::string depth_label = "avg depth: " + std::to_string(avg_depth) + " mm";
				cv::Point label_pt(711, 380);
				cv::putText(shown_img, depth_label, label_pt, cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, cv::Scalar(white));
				
				depth_label = "stddev: " + std::to_string(depth_planarity_value) + " mm";
				label_pt.y += 15;
				cv::putText(shown_img, depth_label, label_pt, cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, cv::Scalar(white));
			}
		}

		cv::imshow(window_name, shown_img);
		
		cv::waitKey(1);	
	}
	
	device->stop();
	device->close();
}
