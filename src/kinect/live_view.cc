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

using namespace tlz;


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
	cv::namedWindow(window_name, CV_WINDOW_NORMAL | CV_WINDOW_FREERATIO);
	int max_depth = 6000; // mm
	int z_near_slider_value = 0, z_far_slider_value = max_depth;
	cv::createTrackbar("min depth (mm)", window_name, &z_near_slider_value, max_depth);
	cv::createTrackbar("max depth (mm)", window_name, &z_far_slider_value, max_depth);

	Frame undistorted_depth(depth_width, depth_height, 4);
	Frame registered_texture(depth_width, depth_height, 4);
	Frame upscaled_depth(1920, 1082, 4);
	Registration registration(ir, color);
	
	cv::Mat_<cv::Vec3b> shown_img(texture_height, texture_width*2);

	bool continuing = true;
	while(continuing) {
		ok = listener.waitForNewFrame(frames, 10*1000);
		if(! ok) break;
		
		Frame* raw_texture = frames[Frame::Color];
		Frame* raw_depth = frames[Frame::Depth];
				
		registration.apply(raw_texture, raw_depth, &undistorted_depth, &registered_texture, true, &upscaled_depth);

		cv::Mat_<float> depth_orig(1082, 1920, reinterpret_cast<float*>(upscaled_depth.data));		
		cv::Mat_<ushort> depth = depth_orig.rowRange(1, 1081);
		
		ushort z_near = z_near_slider_value;
		ushort z_far = z_far_slider_value;
		float alpha = 255.0 / (z_far - z_near);
		float beta = -alpha * z_near;

		cv::Mat_<uchar> scaled_depth;
		cv::convertScaleAbs(depth, scaled_depth, alpha, beta);
		scaled_depth.setTo(0, (depth < z_near));
		scaled_depth.setTo(255, (depth > z_far));
		scaled_depth.setTo(0, (depth == 0));
		cv::Mat_<cv::Vec3b> shown_depth;
		cv::cvtColor(scaled_depth, shown_depth, CV_GRAY2BGR);	
				
		cv::Mat_<cv::Vec4b> texture_orig(texture_height, texture_width, reinterpret_cast<cv::Vec4b*>(raw_texture->data));
		cv::Mat_<cv::Vec3b> shown_texture;
		cv::cvtColor(texture_orig, shown_texture, CV_BGRA2BGR);

		shown_texture.copyTo(cv::Mat(shown_img, cv::Rect(0, 0, texture_width, texture_height)));
		shown_depth.copyTo(cv::Mat(shown_img, cv::Rect(texture_width-1, 0, texture_width, texture_height)));
		listener.release(frames);
	
		cv::imshow(window_name, shown_img);
		
		cv::waitKey(1);	
	}
	
	device->stop();
	device->close();
}
