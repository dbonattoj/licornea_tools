#ifndef LICORNEA_KINECT_LIVE_GRABBER_H_
#define LICORNEA_KINECT_LIVE_GRABBER_H_

#include "../../lib/opencv.h"
#include "../freenect2.h"
#include "../kinect_internal_parameters.h"
#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/registration.h>
#include <libfreenect2/packet_pipeline.h>
#include <libfreenect2/logger.h>
#include <string>
#include <memory>
#include <vector>

namespace tlz {

class grabber {
public:
	enum frame_type_value {
		color = 1<<0,
		registered_color = 1<<1,
		depth = 1<<2,
		bigdepth = 1<<3,
		ir = 1<<4
	};
			
private:	
	int frame_types_;
	bool released_;
	
	libfreenect2::Freenect2 context_;
	libfreenect2::CpuPacketPipeline* pipeline_;
	libfreenect2::SyncMultiFrameListener listener_;
	libfreenect2::FrameMap frames_;
	libfreenect2::Freenect2Device* device_;
	std::unique_ptr<libfreenect2::Registration> registration_;
	libfreenect2::Frame undistorted_depth_;
	libfreenect2::Frame undistorted_ir_;
	libfreenect2::Frame registered_color_;
	libfreenect2::Frame bigdepth_;

	bool has_(frame_type_value t) const { return ((frame_types_ & t) != 0); }
	int freenect2_frame_types_() const;
	
public:	
	explicit grabber(int frame_types);
	~grabber();
	
	bool grab();
	void release();
	
	libfreenect2::Freenect2& context() { return context_; }
	libfreenect2::Freenect2Device& device() { return *device_; }
	libfreenect2::Registration& registration() { return *registration_; }
	kinect_internal_parameters internal_parameters();
	
	cv::Mat_<cv::Vec3b> get_color_frame();
	cv::Mat_<cv::Vec3b> get_registered_color_frame();
	cv::Mat_<uchar> get_ir_frame(float min_ir = 0, float max_ir = 0xffff, bool undistorted = false);
	cv::Mat_<float> get_depth_frame(bool undistorted = false);
	cv::Mat_<float> get_bigdepth_frame();
};

}

#endif
