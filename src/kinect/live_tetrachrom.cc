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



int main(int argc, const char* argv[]) {
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
	device->setColorFrameListener(&listener);
	device->setIrAndDepthFrameListener(&listener);

	bool ok = device->start();
	if(! ok) throw std::runtime_error("could not start device");

	Freenect2Device::ColorCameraParams color = device->getColorCameraParams();
	Freenect2Device::IrCameraParams ir = device->getIrCameraParams();

	std::string window_name = "Viewer";
	cv::namedWindow(window_name, CV_WINDOW_AUTOSIZE);

	int hue_start = 100, ir_min = 0, ir_max = 0xffff;
	cv::createTrackbar("hue start", window_name, &hue_start, 255.0);
	cv::createTrackbar("min ir", window_name, &ir_min, 0xffff);
	cv::createTrackbar("max ir", window_name, &ir_max, 0xffff);

	Frame undistorted_depth(512, 424, 4);
	Frame registered_texture(512, 424, 4);
	Registration registration(ir, color);
	
	cv::Mat_<cv::Vec3b> shown_img(424, 512);

	bool continuing = true;
	while(continuing) {
		ok = listener.waitForNewFrame(frames, 10*1000);
		if(! ok) break;
		
		Frame* raw_texture = frames[Frame::Color];
		Frame* raw_depth = frames[Frame::Depth];
		Frame* raw_ir = frames[Frame::Ir];

		cv::Mat_<cv::Vec3b> visible_bgr;
		cv::Mat_<uchar> ir;

		{
			registration.apply(raw_texture, raw_depth, &undistorted_depth, &registered_texture, true);
			cv::Mat_<cv::Vec4b> texture_orig(424, 512, reinterpret_cast<cv::Vec4b*>(registered_texture.data));
			cv::cvtColor(texture_orig, visible_bgr, CV_BGRA2BGR);
		}

		{
			cv::Mat_<float> ir_orig_float(424, 512, reinterpret_cast<float*>(raw_ir->data));		
			cv::Mat_<ushort> ir_orig = ir_orig_float;
		
			ir = scale_grayscale(ir_orig, ir_min, ir_max);
		}

		listener.release(frames);

		cv::Mat_<cv::Vec3b> visible_hsv;
		cv::Mat_<uchar> visible_hue;
		cv::cvtColor(visible_bgr, visible_hsv, CV_BGR2HSV);
		cv::extractChannel(visible_hsv, visible_hue, 0);
		
		cv::Mat_<uchar> hue;
		{
			real hue_visible_start = hue_start/255.0;
			cv::Mat_<real> v = visible_hue;
			cv::Mat_<real> i = ir;
			
			cv::Mat_<real> vi = v * (1.0-hue_visible_start) + hue_visible_start*255.0;
			vi -= i * hue_visible_start;
			hue = vi;
		}
		

		cv::Mat_<cv::Vec3b> hsv(424, 512);
		visible_hsv.copyTo(hsv);
		const int fromTo[] = {0, 0};	
		cv::mixChannels(&hue, 1, &hsv, 1, fromTo, 1);

		cv::cvtColor(hsv, shown_img, CV_HSV2BGR);

		cv::Vec3b black(0,0,0);
		cv::Mat_<uchar> holes;
		cv::inRange(shown_img, black, black, holes);
		cv::inpaint(shown_img, holes, shown_img, 4, cv::INPAINT_TELEA);

		cv::imshow(window_name, shown_img);
		
		cv::waitKey(1);	
	}
	
	device->stop();
	device->close();
}
