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
const cv::Vec3b indicator2_color = cv::Vec3b(255,0,0);

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

void draw_checkerboard(cv::Mat_<cv::Vec3b>& image, int cols, int rows, cv::vector<cv::Point2f>& corners, cv::Vec3b col) {
	std::vector<cv::Point> outer_corners = checkerboard_outer_corners(cols, rows, corners);
	std::vector<std::vector<cv::Point>> polylines { outer_corners };
	
	cv::polylines(image, polylines, true, cv::Scalar(col), 1);
	
	vec2 centroid = checkerboard_centroid(cols, rows, corners);
	cv::circle(image, cv::Point(centroid), 7, cv::Scalar(col), 2);
}


[[noreturn]] void usage_fail() {
	std::cout << "usage: live_checkerboard_distance cols rows square_width\n";
	std::cout << std::endl;
	std::exit(1);
}
int main(int argc, const char* argv[]) {
	if(argc <= 3) usage_fail();
	const int cols = std::stoi(argv[1]);
	const int rows = std::stoi(argv[2]);
	const real square_width = std::stof(argv[3]);
		
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
	int max_depth = 6000; // mm
	int z_near = 0, z_far = max_depth;
	int z_range = 150, ir_min = 0, ir_max = 0xffff;
	cv::createTrackbar("visual depth range (mm)", window_name, &z_range, 2000);
	cv::createTrackbar("IR min", window_name, &ir_min, 0xffff);
	cv::createTrackbar("IR max", window_name, &ir_max, 0xffff);

	Frame undistorted_depth(512, 424, 4);
	Frame undistorted_ir(512, 424, 4);
		
	cv::Mat_<cv::Vec3b> shown_img(424 + 30, 512*2);
	cv::Mat_<float> depth_frame(424, 512);
	cv::Mat_<uchar> ir_frame(424, 512);
	cv::Mat_<uchar> checkerboard_depth_mask(424, 512);

	Freenect2Device::ColorCameraParams color_param = device->getColorCameraParams();
	Freenect2Device::IrCameraParams ir_param = device->getIrCameraParams();
	Registration registration(ir_param, color_param);
	mat33 intrinsic(
		ir_param.fx, 0.0, ir_param.cx,
		0.0, ir_param.fy, ir_param.cy,
		0.0, 0.0, 1.0
	);

	bool continuing = true;
	while(continuing) {
		// acquire frames (depth + color)
		ok = listener.waitForNewFrame(frames, 10*1000);
		if(! ok) break;
		Frame* raw_ir = frames[Frame::Ir];
		Frame* raw_depth = frames[Frame::Depth];

		// undistort depth + ir
 		registration.undistortDepth(raw_depth, &undistorted_depth);
		registration.undistortDepth(raw_ir, &undistorted_ir);
		
		// read depth+ir frame
		{
 			registration.undistortDepth(raw_depth, &undistorted_depth);
			depth_frame = cv::Mat_<float>(424, 512, reinterpret_cast<float*>(undistorted_depth.data));		
			depth_frame.setTo(0, depth_frame == INFINITY);

			registration.undistortDepth(raw_ir, &undistorted_ir);
			cv::Mat_<float> ir_orig(424, 512, reinterpret_cast<float*>(undistorted_ir.data));		
			cv::Mat_<ushort> ir = ir_orig;
			ir_frame = scale_grayscale(ir, ir_min, ir_max);
 			
			listener.release(frames);
		}
		
		listener.release(frames);

		std::vector<cv::Point2f> corners;
		cv::Size pattern_size(cols, rows);

		// detect checkerboard
		int flags = 0;
		bool found_checkerboard = cv::findChessboardCorners(ir_frame, pattern_size, corners, flags);
		if(corners.size() != rows*cols) found_checkerboard = false;
			
		cv::vector<cv::Point2f> measured_depth_corners;
		real min_d = z_near, max_d = z_far;
		cv::Rect checkerboard_rect(0, 0, 512, 424);
		
		real projection_depth = NAN;
		cv::Point reprojected_centroid;
		
		real sensor_depth = NAN;
			
		checkerboard_depth_mask.setTo(0);
		if(found_checkerboard) {
			// refine checkerboard
			{
				cv::TermCriteria term(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30, 0.1);
				cv::cornerSubPix(ir_frame, corners, cv::Size(3, 3), cv::Size(-1, -1), term);
			}
			
			// extimate extrinsic from checkerboard
			mat33 rotation; vec3 translation;
			{
				std::vector<vec3> object_points(rows * cols);
				std::vector<vec2> image_points(rows * cols);
				for(int row = 0, i = 0; row < rows; ++row) for(int col = 0; col < cols; ++col, ++i) {
					cv::Point2f corner = corners[i];
					image_points[i] = vec2(corner.x, corner.y);
					object_points[i] = vec3(col*square_width, row*square_width, 0.0);
				}
				
				vec3 rotation_vec, translation_vec;
				cv::Mat distortion;
								
				cv::solvePnP(
					object_points,
					image_points,
					intrinsic,
					distortion,
					rotation_vec,
					translation,
					false
				);
								
				mat33 rotation_mat;
				cv::Rodrigues(rotation_vec, rotation);
			}
			
			
			// project checkerboard centroid, and get its orthogonal distance (= projection depth)
			vec3 world_centroid(square_width*(cols-1)/2.0, square_width*(rows-1)/2.0, 0.0);
			vec2 image_centroid = checkerboard_centroid(cols, rows, corners);
			
			{
				vec3 w = world_centroid;
				vec3 v = rotation*w + translation;
				projection_depth = v[2];
				vec3 i_h = intrinsic * v;
				vec2 i(i_h[0]/i_h[2], i_h[1]/i_h[2]);
				reprojected_centroid = cv::Point(i);
			}
	
			
			// mask and bounding rectangle for depth map of checkerboard
			std::vector<cv::Point> outer_corners = checkerboard_outer_corners(cols, rows, corners);
			cv::fillConvexPoly(checkerboard_depth_mask, outer_corners.data(), 4, cv::Scalar(255));
			checkerboard_depth_mask.setTo(0, (depth_frame == 0.0));

			int min_x = 512, max_x = 0, min_y = 424, max_y = 0;
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
			
			// get minimal, maximal, average depth of checkerboard
			real depth_sum = 0.0;
			int depth_count = 0;
			min_d = INFINITY; max_d = 0.0;
			for(int y = min_y; y <= max_y; ++y) for(int x = min_x; x <= max_x; ++x) {
				uchar mask = checkerboard_depth_mask(y, x);
				if(! mask) continue;
				real d = depth_frame(y, x);
				depth_sum += d;
				depth_count++;
				if(d < min_d) min_d = d;
				if(d > max_d) max_d = d;
			}
			sensor_depth = depth_sum / depth_count; // sensor depth = average of depth values
			
			// get min_d and max_d for visualization
			real mid_d = (max_d + min_d)/2.0;
			min_d = mid_d - z_range/2.0;
			max_d = mid_d + z_range/2.0;
		}

	
		{
			shown_img.setTo(black);
			
			cv::Mat_<cv::Vec3b> ir, depth;
			cv::cvtColor(ir_frame, ir, CV_GRAY2BGR);
			
			if(found_checkerboard) {
				draw_checkerboard(ir, cols, rows, corners, indicator_color);
				cv::circle(ir, reprojected_centroid, 7, cv::Scalar(indicator2_color), 2);
			}
			ir.copyTo(cv::Mat(shown_img, cv::Rect(0, 0, 512, 424)));
			
			cv::Mat_<uchar> depth_uchar = scale_grayscale(depth_frame, min_d, max_d);
			cv::cvtColor(depth_uchar, depth, CV_GRAY2BGR);
			
			if(found_checkerboard) draw_checkerboard(depth, cols, rows, corners, indicator_color);
			cv::resize(depth, depth, cv::Size(512, 424));
			depth.copyTo(cv::Mat(shown_img, cv::Rect(512, 0, 512, 424)));

			int font = cv::FONT_HERSHEY_COMPLEX_SMALL;
			double fontscale = 0.8;
			int thickness = 1;

			// left label
			{
				std::string label = "projection depth: " + std::to_string(projection_depth) + " mm";
				cv::Point pt(20, 424+20);
				cv::putText(shown_img, label, pt, font, fontscale, cv::Scalar(white), thickness);
			}
	
	
			// right label
			{
				std::string label = "sensor depth: " + std::to_string(sensor_depth) + " mm";
				cv::Size sz = cv::getTextSize(label, font, fontscale, thickness, nullptr);
				cv::Point pt(512*2-20-sz.width, 424+20);
				cv::putText(shown_img, label, pt, font, fontscale, cv::Scalar(white), thickness);
			}
			
			// center label
			{
				std::string label = "diff = " + std::to_string(projection_depth - sensor_depth) + " mm";
				cv::Size sz = cv::getTextSize(label, font, fontscale, thickness, nullptr);
				cv::Point pt(512-sz.width/2, 424+20);
				cv::putText(shown_img, label, pt, font, fontscale, cv::Scalar(white), thickness);
			}
		}

		cv::imshow(window_name, shown_img);
		
		cv::waitKey(1);	
	}
	
	device->stop();
	device->close();
}
