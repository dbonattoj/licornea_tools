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



int main(int argc, const char* argv[]) {
	using namespace libfreenect2;

	Freenect2 context;
	int count = context.enumerateDevices();
	if(count == 0) throw std::runtime_error("Kinect not found");
		
	CpuPacketPipeline pipeline;
	
	std::string serial = context.getDefaultDeviceSerialNumber();
	std::unique_ptr<Freenect2Device> device(context.openDevice(serial, &pipeline));
	if(device == nullptr) throw std::runtime_error("could not open device");
	
	int types = Frame::Ir | Frame::Depth;
	SyncMultiFrameListener listener(types);
	FrameMap frames;
	device->setIrAndDepthFrameListener(&listener);

	bool ok = device->start();
	if(! ok) throw std::runtime_error("could not start device");

	std::string window_name = "Viewer";
	cv::namedWindow(window_name, CV_WINDOW_AUTOSIZE);
	int z_range = 500;
	int points_border = 20;
	int ir_brightness = 100;
	int indicator_factor = 30;
	cv::createTrackbar("visual range (mm)", window_name, &z_range, 2000);
	cv::createTrackbar("IR brightness", window_name, &ir_brightness, 500);
	cv::createTrackbar("border (pixel)", window_name, &points_border, 200);
	cv::createTrackbar("indicator precision", window_name, &indicator_factor, 200);
	
	const cv::Vec3b depth_indicator_color = cv::Vec3b(100,100,255);
	const real stddev_depth_max = 20;
	
	cv::Mat_<cv::Vec3b> shown_img(424 + 24, 512*2);
	cv::Mat_<float> depth_frame(424, 512);
	cv::Mat_<uchar> ir_frame(424, 512);

	bool continuing = true;
	while(continuing) {
		ok = listener.waitForNewFrame(frames, 10*1000);
		if(! ok) break;

		Frame* raw_ir = frames[Frame::Ir];
		Frame* raw_depth = frames[Frame::Depth];
				
		{
			depth_frame = cv::Mat_<float>(424, 512, reinterpret_cast<float*>(raw_depth->data));		
			depth_frame.setTo(0, depth_frame == INFINITY);
		}

		{
			cv::Mat_<float> ir_orig(424, 512, reinterpret_cast<float*>(raw_ir->data));		
			cv::Mat_<ushort> ir = ir_orig;
			ir_frame = scale_grayscale(ir, 0, 0xffff);
		}
		
		listener.release(frames);
		
		
		int min_x = points_border, max_x = 512 - points_border;
		int min_y = points_border, max_y = 424 - points_border;
		int x_count = max_x - min_x + 1, y_count = max_y - min_y + 1;
		
		real min_d = INFINITY;
		real max_d = 0.0;
		real avg_depth;
		real stddev_depth;
		real hslope, vslope;
		{
			cv::vector<real> depth_samples;
			std::vector<real> hmeans(x_count, 0.0), vmeans(y_count, 0.0);
			std::vector<int> hcounts(x_count, 0), vcounts(y_count, 0);
			for(int y = min_y; y <= max_y; ++y) for(int x = min_x; x <= max_x; ++x) {
				real d = depth_frame(y, x);
				if(d == 0.0) continue;
		
				depth_samples.push_back(d);
				if(d < min_d) min_d = d;
				if(d > max_d) max_d = d;
				
				hmeans[x-min_x] += d;
				hcounts[x-min_x]++;
				vmeans[y-min_y] += d;
				vcounts[y-min_y]++;
			}
			avg_depth = NAN;
			stddev_depth = stddev(depth_samples, &avg_depth);
			real mid_d = (max_d + min_d)/2.0;
			min_d = mid_d - z_range/2.0;
			max_d = mid_d + z_range/2.0;		
			
			
			for(int x = min_x; x <= max_x; ++x) hmeans[x-min_x] /= hcounts[x-min_x];
			for(int y = min_y; y <= max_y; ++y) vmeans[y-min_y] /= vcounts[y-min_y];

			{
				std::vector<cv::Vec2f> points;
				for(int x = min_x; x <= max_x; ++x) points.emplace_back(x, hmeans[x-min_x]);
				cv::Vec4f params;
				cv::fitLine(points, params, CV_DIST_L2, 0, 0.01, 0.01);
				hslope = params[1] / params[0];
			}
			{
				std::vector<cv::Vec2f> points;
				for(int y = min_y; y <= max_y; ++y) points.emplace_back(y, vmeans[y-min_y]);
				cv::Vec4f params;
				cv::fitLine(points, params, CV_DIST_L2, 0, 0.01, 0.01);
				vslope = params[1] / params[0];
			}
		}
	
		{
			cv::Mat_<cv::Vec3b> ir, depth;
			shown_img.setTo(black);
			
			cv::Mat_<uchar> depth_uchar = scale_grayscale(depth_frame, min_d, max_d);
			cv::cvtColor(depth_uchar, depth, CV_GRAY2BGR);
			
			ir_frame *= ir_brightness/100.0;
			cv::cvtColor(ir_frame, ir, CV_GRAY2BGR);
			int dot_rad = 7, dot_width = 2;
			std::vector<cv::Point> dots {
				cv::Point(min_x, min_y),
				cv::Point(min_x, max_y),
				cv::Point(max_x, min_y),
				cv::Point(max_x, max_y)
			};
			for(const cv::Point& dot : dots) {
				cv::circle(ir, dot, dot_rad, cv::Scalar(depth_indicator_color), dot_width);
				cv::circle(depth, dot, dot_rad, cv::Scalar(depth_indicator_color), dot_width);
			}
			
			ir.copyTo(cv::Mat(shown_img, cv::Rect(0, 0, 512, 424)));
			depth.copyTo(cv::Mat(shown_img, cv::Rect(512, 0, 512, 424)));

			std::string depth_label = "avg depth: " + std::to_string(avg_depth) + " mm";
			cv::Point label_pt(20, 440);
			cv::putText(shown_img, depth_label, label_pt, cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, cv::Scalar(white));
			
			depth_label = "stddev: " + std::to_string(stddev_depth) + " mm";
			label_pt.x += 512;
			cv::putText(shown_img, depth_label, label_pt, cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, cv::Scalar(white));
			
			std::vector<std::vector<cv::Point>> polylines;
			polylines.push_back({ cv::Point(512+512/2, 0), cv::Point(512+512/2, 424) });
			polylines.push_back({ cv::Point(512, 424/2), cv::Point(512+512, 424/2) });
			cv::polylines(shown_img, polylines, false, cv::Scalar(black), 3);
			cv::polylines(shown_img, polylines, false, cv::Scalar(white), 1);
			
			const real factor = indicator_factor;
			const int indicator_rad = 20;
			int indicator_x = 512+512/2 + hslope*factor;
			int indicator_y = 424/2 + vslope*factor;
			polylines.clear();
			polylines.push_back({ cv::Point(indicator_x-indicator_rad, indicator_y), cv::Point(indicator_x+indicator_rad, indicator_y) });
			polylines.push_back({ cv::Point(indicator_x, indicator_y-indicator_rad), cv::Point(indicator_x, indicator_y+indicator_rad) });
			cv::polylines(shown_img, polylines, false, cv::Scalar(depth_indicator_color), 3);
		}

		cv::imshow(window_name, shown_img);
		
		cv::waitKey(1);	
	}
	
	device->stop();
	device->close();
}

