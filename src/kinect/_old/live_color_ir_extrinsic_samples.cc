#include <json.hpp>
#include "lib/common.h"
#include "../lib/opencv.h"
#include "../lib/json.h"
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
const cv::Vec3b indicator_color = cv::Vec3b(0,0,255);


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

std::vector<cv::Point> checkerboard_outer_corners(int cols, int rows, const std::vector<cv::Point2f>& corners) {
	return {
		corners.at(0),
		corners.at(cols-1),
		corners.at(cols*(rows-1) + (cols-1)),
		corners.at(cols*(rows-1))
	};
}

vec2 checkerboard_centroid(int cols, int rows, const std::vector<cv::Point2f>& corners) {
	std::vector<cv::Point> outer_corners = checkerboard_outer_corners(cols, rows, corners);
	real centroid_x = (outer_corners[0].x + outer_corners[1].x + outer_corners[2].x + outer_corners[3].x) / 4.0;
	real centroid_y = (outer_corners[0].y + outer_corners[1].y + outer_corners[2].y + outer_corners[3].y) / 4.0;
	return vec2(centroid_x, centroid_y);
}

void draw_checkerboard(cv::Mat_<cv::Vec3b>& image, int cols, int rows, cv::vector<cv::Point2f>& corners, cv::Vec3b col, int width) {
	std::vector<cv::Point> outer_corners = checkerboard_outer_corners(cols, rows, corners);
	std::vector<std::vector<cv::Point>> polylines { outer_corners };
	
	cv::polylines(image, polylines, true, cv::Scalar(col), width);
}


[[noreturn]] void usage_fail() {
	std::cout << "usage: live_color_ir_extrinsic_samples cols rows square_width color_intrinsics.json\n";
	std::cout << std::endl;
	std::exit(1);
}
int main(int argc, const char* argv[]) {
	if(argc <= 4) usage_fail();
	const int cols = std::stoi(argv[1]);
	const int rows = std::stoi(argv[2]);
	const real square_width = std::stof(argv[3]);
	std::string intrinsics_filename = argv[4];
	
	mat33 intrinsic = decode_mat(import_json_file(intrinsics_filename)["K"]);

	using namespace libfreenect2;

	Freenect2 context;
	int count = context.enumerateDevices();
	if(count == 0) throw std::runtime_error("Kinect not found");
		
	CpuPacketPipeline pipeline;
	
	std::string serial = context.getDefaultDeviceSerialNumber();
	std::unique_ptr<Freenect2Device> device(context.openDevice(serial, &pipeline));
	if(device == nullptr) throw std::runtime_error("could not open device");
	
	int types = Frame::Color | Frame::Ir | Frame::Depth;
	SyncMultiFrameListener listener(types);
	FrameMap frames;
	device->setIrAndDepthFrameListener(&listener);
	device->setColorFrameListener(&listener);

	bool ok = device->start();
	if(! ok) throw std::runtime_error("could not start device");

	std::string window_name = "Viewer";
	cv::namedWindow(window_name, CV_WINDOW_AUTOSIZE);
	int z_range = 500, ir_min = 0, ir_max = 0xffff;
	cv::createTrackbar("visual depth range (mm)", window_name, &z_range, 2000);
	cv::createTrackbar("IR min", window_name, &ir_min, 0xffff);
	cv::createTrackbar("IR max", window_name, &ir_max, 0xffff);
		
	cv::Mat_<cv::Vec3b> shown_img(424 + 24, 754+512+512);
	cv::Mat_<float> depth_frame(424, 512);
	cv::Mat_<uchar> ir_frame(424, 512);
	cv::Mat_<cv::Vec3b> texture_frame(1080, 1920);

	Frame undistorted_ir(512, 424, 4);
	Frame undistorted_depth(512, 424, 4);

	Freenect2Device::ColorCameraParams color = device->getColorCameraParams();
	Freenect2Device::IrCameraParams ir = device->getIrCameraParams();
	Registration registration(ir, color);

	bool continuing = true;
	while(continuing) {
		ok = listener.waitForNewFrame(frames, 10*1000);
		if(! ok) break;

		Frame* raw_texture = frames[Frame::Color];
		Frame* raw_ir = frames[Frame::Ir];
		Frame* raw_depth = frames[Frame::Depth];

		// read texture (color), ir, depth frame
		{		
			cv::Mat_<cv::Vec4b> texture_orig(1080, 1920, reinterpret_cast<cv::Vec4b*>(raw_texture->data));
			cv::cvtColor(texture_orig, texture_frame, CV_BGRA2BGR);

 			registration.undistortDepth(raw_depth, &undistorted_depth);
			depth_frame = cv::Mat_<float>(424, 512, reinterpret_cast<float*>(undistorted_depth.data));		
			depth_frame.setTo(0, depth_frame == INFINITY);

			registration.undistortDepth(raw_ir, &undistorted_ir);
			cv::Mat_<float> ir_orig(424, 512, reinterpret_cast<float*>(undistorted_ir.data));		
			cv::Mat_<ushort> ir = ir_orig;
			ir_frame = scale_grayscale(ir, ir_min, ir_max);
 			
			listener.release(frames);
		}
		
		
		

		real min_d = 0, max_d = 6000;

		
		
		
		// detect checkerboards
		bool found_texture_checkerboard = false, found_ir_checkerboard = false;
		std::vector<cv::Point2f> texture_corners;
		std::vector<cv::Point2f> ir_corners;
		{	
			cv::Size pattern_size(cols, rows);
		
			int flags = 0;
			found_ir_checkerboard = cv::findChessboardCorners(ir_frame, pattern_size, ir_corners, flags);
			if(ir_corners.size() != rows*cols) found_ir_checkerboard = false;
			
			flags = 0;
			found_texture_checkerboard = cv::findChessboardCorners(texture_frame, pattern_size, texture_corners, flags);
			if(texture_corners.size() != rows*cols) found_texture_checkerboard = false;
		}
		
		
		
		
		
		{
			cv::Mat_<cv::Vec3b> texture, ir, depth;
			shown_img.setTo(black);
			
			if(found_texture_checkerboard) draw_checkerboard(texture_frame, cols, rows, texture_corners, indicator_color, 5);
			cv::resize(texture_frame, texture, cv::Size(754, 424));
			texture.copyTo(cv::Mat(shown_img, cv::Rect(0, 0, 754, 424)));
													
						
			cv::cvtColor(ir_frame, ir, CV_GRAY2BGR);
			if(found_ir_checkerboard) draw_checkerboard(ir, cols, rows, ir_corners, indicator_color, 2);
			ir.copyTo(cv::Mat(shown_img, cv::Rect(754, 0, 512, 424)));

			cv::Mat_<uchar> depth_uchar = scale_grayscale(depth_frame, min_d, max_d);
			cv::cvtColor(depth_uchar, depth, CV_GRAY2BGR);
			if(found_ir_checkerboard) draw_checkerboard(depth, cols, rows, ir_corners, indicator_color, 2);
			depth.copyTo(cv::Mat(shown_img, cv::Rect(754+512, 0, 512, 424)));			
		}

		cv::imshow(window_name, shown_img);
		
		cv::waitKey(1);	
	}
	
	device->stop();
	device->close();
}

