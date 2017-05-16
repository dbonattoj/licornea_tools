#ifdef LICORNEA_WITH_LIBFREENECT2

#include "grabber.h"
#include <cassert>

namespace tlz {

using namespace libfreenect2;

int grabber::freenect2_frame_types_() const {
	int types = 0;
	if(has_(color) || has_(registered_color)) types |= Frame::Color;
	if(has_(ir)) types |= Frame::Ir;
	if(has_(depth) || has_(bigdepth) || has_(registered_color)) types |= Frame::Depth;
	return types;
}

grabber::grabber(int frame_types) :
	frame_types_(frame_types),
	released_(true),
	context_(),
	pipeline_(),
	listener_(freenect2_frame_types_()),
	frames_(),
	undistorted_depth_(512, 424, 4),
	undistorted_ir_(512, 424, 4),
	registered_color_(512, 424, 4),
	bigdepth_(1920, 1082, 4)
{
	setGlobalLogger(nullptr);
	
	int device_count = context_.enumerateDevices();
	if(device_count == 0) throw std::runtime_error("Kinect not found");

	std::string serial = context_.getDefaultDeviceSerialNumber();
	device_.reset(context_.openDevice(serial, &pipeline_));
	if(! device_) throw std::runtime_error("could not open device");

	if(has_(color) || has_(registered_color))
		device_->setColorFrameListener(&listener_);
	if(has_(registered_color) || has_(depth) || has_(depth) || has_(bigdepth) || has_(ir))
		device_->setIrAndDepthFrameListener(&listener_);
	
	bool ok = device_->start();
	if(! ok) throw std::runtime_error("could not start device");

	Freenect2Device::ColorCameraParams color = device_->getColorCameraParams();
	Freenect2Device::IrCameraParams ir = device_->getIrCameraParams();
	registration_ = std::make_unique<Registration>(ir, color);
}	


grabber::~grabber() {
	device_->stop();
	device_->close();
}


bool grabber::grab() {
	const int max_wait_ms = 5000;
	bool ok = listener_.waitForNewFrame(frames_, max_wait_ms);
	if(! ok) return false;
	released_ = false;
	
	if(has_(registered_color) || has_(bigdepth)) {
		Frame* raw_color = frames_[Frame::Color];
		Frame* raw_depth = frames_[Frame::Depth];
		registration_->apply(raw_color, raw_depth, &undistorted_depth_, &registered_color_, true, &bigdepth_);
	} else if(has_(depth)) {
		Frame* raw_depth = frames_[Frame::Depth];
		registration_->undistortDepth(raw_depth, &undistorted_depth_);
	}
	if(has_(ir)) {
		Frame* raw_ir = frames_[Frame::Ir];
		registration_->undistortDepth(raw_ir, &undistorted_ir_);
	}
	
	return true;
}


void grabber::release() {
	if(! released_) listener_.release(frames_);
	released_ = true;
}


	
cv::Mat_<cv::Vec3b> grabber::get_color_frame() {
	assert(has_(color));
	Frame* raw_color = frames_[Frame::Color];
	cv::Mat_<cv::Vec4b> color_orig(1080, 1920, reinterpret_cast<cv::Vec4b*>(raw_color->data));
	cv::Mat_<cv::Vec3b> color;
	cv::cvtColor(color_orig, color, CV_BGRA2BGR);
	return color;
}


cv::Mat_<cv::Vec3b> grabber::get_registered_color_frame() {
	assert(has_(registered_color));
	cv::Mat_<cv::Vec4b> color_orig(424, 512, reinterpret_cast<cv::Vec4b*>(registered_color_.data));
	cv::Mat_<cv::Vec3b> color;
	cv::cvtColor(color_orig, color, CV_BGRA2BGR);
	return color;
}


cv::Mat_<uchar> grabber::get_ir_frame(float min_ir, float max_ir) {
	cv::Mat_<float> ir_orig(424, 512, reinterpret_cast<float*>(undistorted_ir_.data));		
	float alpha = 255.0f / (max_ir - min_ir);
	float beta = -alpha * min_ir;
	cv::Mat_<uchar> ir;
	cv::convertScaleAbs(ir_orig, ir, alpha, beta);
	ir.setTo(0, (ir_orig < min_ir));
	ir.setTo(255, (ir_orig > max_ir));
	ir.setTo(0, (ir_orig == 0));
	return ir;
}


cv::Mat_<float> grabber::get_depth_frame() {
	return cv::Mat_<float>(424, 512, reinterpret_cast<float*>(undistorted_depth_.data));		
}


cv::Mat_<float> grabber::get_bigdepth_frame() {
	cv::Mat_<float> depth_orig(1082, 1920, reinterpret_cast<float*>(bigdepth_.data));		
	return depth_orig.rowRange(1, 1081);
}


}

#endif
