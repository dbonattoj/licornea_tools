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
	
	int types = Frame::Color | Frame::Depth | Frame::Ir;
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
	cv::createTrackbar("min depth (mm)", window_name, &z_near, max_depth);
	cv::createTrackbar("max depth (mm)", window_name, &z_far, max_depth);

	Frame undistorted_depth(512, 424, 4);
	Frame registered_texture(512, 424, 4);
	Frame upscaled_depth(1920, 1082, 4);
	Registration registration(ir, color);
	
	cv::Mat_<cv::Vec3b> shown_img(400*2, 711*2);

	bool continuing = true;
	while(continuing) {
		ok = listener.waitForNewFrame(frames, 10*1000);
		if(! ok) break;
		

		Frame* raw_texture = frames[Frame::Color];
		Frame* raw_depth = frames[Frame::Depth];
		Frame* raw_ir = frames[Frame::Ir];
				
		{
			registration.apply(raw_texture, raw_depth, &undistorted_depth, &registered_texture, true, &upscaled_depth);

			cv::Mat_<float> rdepth_orig(1082, 1920, reinterpret_cast<float*>(upscaled_depth.data));		
			cv::Mat_<ushort> rdepth = rdepth_orig.rowRange(1, 1081);
		
			cv::Mat_<uchar> scaled_rdepth = scale_grayscale(rdepth, z_near, z_far);
			cv::Mat_<cv::Vec3b> shown_rdepth;
			cv::cvtColor(scaled_rdepth, shown_rdepth, CV_GRAY2BGR);	
			cv::resize(shown_rdepth, shown_rdepth, cv::Size(711, 400));
				
			shown_rdepth.copyTo(cv::Mat(shown_img, cv::Rect(711, 0, 711, 400)));
		}


		{
			cv::Mat_<cv::Vec4b> texture_orig(1080, 1920, reinterpret_cast<cv::Vec4b*>(raw_texture->data));
			cv::Mat_<cv::Vec3b> shown_texture;
			cv::cvtColor(texture_orig, shown_texture, CV_BGRA2BGR);
			cv::resize(shown_texture, shown_texture, cv::Size(711, 400));
			
			shown_texture.copyTo(cv::Mat(shown_img, cv::Rect(0, 0, 711, 400)));
		}


		{
			cv::Mat_<float> depth_orig(424, 512, reinterpret_cast<float*>(raw_depth->data));		
			cv::Mat_<ushort> depth = depth_orig;
		
			cv::Mat_<uchar> scaled_depth = scale_grayscale(depth, z_near, z_far);
			cv::Mat_<cv::Vec3b> shown_depth;
			cv::cvtColor(scaled_depth, shown_depth, CV_GRAY2BGR);	
			cv::resize(shown_depth, shown_depth, cv::Size(483, 400));
				
			shown_depth.copyTo(cv::Mat(shown_img, cv::Rect(711 + (711-483)/2, 400, 483, 400)));
		}


		{
			cv::Mat_<float> ir_orig(424, 512, reinterpret_cast<float*>(raw_ir->data));		
			cv::Mat_<ushort> ir = ir_orig;
		
			cv::Mat_<uchar> scaled_ir = scale_grayscale(ir, 0, 0xffff);
			cv::Mat_<cv::Vec3b> shown_ir;
			cv::cvtColor(scaled_ir, shown_ir, CV_GRAY2BGR);	
			cv::resize(shown_ir, shown_ir, cv::Size(483, 400));
				
			shown_ir.copyTo(cv::Mat(shown_img, cv::Rect(0 + (711-483)/2, 400, 483, 400)));
		}

		listener.release(frames);
	
		cv::imshow(window_name, shown_img);
		
		cv::waitKey(1);	
	}
	
	device->stop();
	device->close();
}
