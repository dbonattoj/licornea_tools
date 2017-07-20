#include "raw_image_io.h"
#include "opencv.h"
#include <fstream>

namespace tlz {
	
namespace {
	
template<typename T>
void read_raw(std::ifstream& stream, T* out, std::size_t length) {
	stream.read(reinterpret_cast<std::ifstream::char_type*>(out), length);
}

template<typename T>
void write_raw(std::ofstream& stream, const T* in, std::size_t length) {
	stream.write(reinterpret_cast<const std::ifstream::char_type*>(in), length);
}



cv::Mat_<cv::Vec3b> import_ycbcr420(std::ifstream& yuv_stream, int width, int height) {
	cv::Size sz(width, height);
	cv::Size sub_sz(width/2, height/2);
	
	cv::Mat_<uchar> y_channel(sz);
	read_raw(yuv_stream, y_channel.data, width*height);

	cv::Mat_<uchar> cb_channel(sub_sz);
	read_raw(yuv_stream, cb_channel.data, width*height/4);
	cv::resize(cb_channel, cb_channel, sz, 0, 0, cv::INTER_CUBIC);

	cv::Mat_<uchar> cr_channel(sub_sz);
	read_raw(yuv_stream, cr_channel.data, width*height/4);
	cv::resize(cr_channel, cr_channel, sz, 0, 0, cv::INTER_CUBIC);

	cv::Mat_<cv::Vec3b> ycrcb;
	std::vector<cv::Mat> src {y_channel, cr_channel, cb_channel};
	cv::merge(src, ycrcb);
	
	cv::Mat_<cv::Vec3b> bgr;
	cv::cvtColor(ycrcb, bgr, CV_YCrCb2BGR);

	return bgr;
}

void export_ycbcr420(std::ofstream& yuv_stream, const cv::Mat_<cv::Vec3b>& bgr) {
	cv::Size sz = bgr.size();
	cv::Size sub_sz(sz.width/2, sz.height/2);
	
	cv::Mat_<cv::Vec3b> ycrcb;
	cv::cvtColor(bgr, ycrcb, CV_BGR2YCrCb);
	
	cv::Mat_<uchar> y_channel(sz), cb_channel(sz), cr_channel(sz);
	std::vector<cv::Mat> dst { y_channel, cr_channel, cb_channel };
	cv::split(ycrcb, dst);
	cv::resize(cb_channel, cb_channel, sub_sz, 0, 0, cv::INTER_CUBIC);
	cv::resize(cr_channel, cr_channel, sub_sz, 0, 0, cv::INTER_CUBIC);
	
	write_raw(yuv_stream, y_channel.data, y_channel.cols*y_channel.rows);
	write_raw(yuv_stream, cb_channel.data, cb_channel.cols*cb_channel.rows);
	write_raw(yuv_stream, cr_channel.data, cr_channel.cols*cr_channel.rows);
}


cv::Mat_<cv::Vec3b> import_rgb_planar(std::ifstream& yuv_stream, int width, int height) {
	cv::Size sz(width, height);
	
	cv::Mat_<uchar> r_channel(sz);
	read_raw(yuv_stream, r_channel.data, width*height);
	
	cv::Mat_<uchar> g_channel(sz);
	read_raw(yuv_stream, g_channel.data, width*height);
	
	cv::Mat_<uchar> b_channel(sz);
	read_raw(yuv_stream, b_channel.data, width*height);

	cv::Mat_<cv::Vec3b> bgr(sz);
	int from_to[] = { 0, 0, 0, 1, 0, 2 };
	std::vector<cv::Mat> dst { bgr };
	std::vector<cv::Mat> src { b_channel, g_channel, r_channel };
	cv::mixChannels(src, dst, from_to, 3);
	
	return bgr;
}

void export_rgb_planar(std::ofstream& yuv_stream, const cv::Mat_<cv::Vec3b>& bgr) {
	cv::Size sz = bgr.size();

	cv::Mat_<uchar> r_channel(sz), g_channel(sz), b_channel(sz);
	std::vector<cv::Mat> dst { b_channel, g_channel, r_channel };
	cv::split(bgr, dst);

	write_raw(yuv_stream, r_channel.data, sz.width*sz.height);
	write_raw(yuv_stream, g_channel.data, sz.width*sz.height);
	write_raw(yuv_stream, b_channel.data, sz.width*sz.height);
}



cv::Mat_<cv::Vec3b> import_rgb_interleaved(std::ifstream& yuv_stream, int width, int height) {
	cv::Size sz(width, height);
	cv::Mat_<uchar> rgb(sz);
	read_raw(yuv_stream, rgb.data, 3*width*height);
	cv::Mat_<uchar> bgr;
	cv::cvtColor(rgb, bgr, CV_RGB2BGR);
	return bgr;
}

void export_rgb_interleaved(std::ofstream& yuv_stream, const cv::Mat_<cv::Vec3b>& bgr) {
	cv::Mat_<cv::Vec3b> rgb;
	cv::cvtColor(bgr, rgb, CV_BGR2RGB);
	write_raw(yuv_stream, rgb.data, 3*rgb.cols*rgb.rows);
}



}

cv::Mat import_raw_color(const std::string& yuv_filename, int width, int height, raw_image_format form) {
	std::ifstream yuv_stream(yuv_filename, std::ios_base::binary);
	switch(form) {
		case raw_image_format::ycbcr420: return import_ycbcr420(yuv_stream, width, height);
		case raw_image_format::rgb_planar: return import_rgb_planar(yuv_stream, width, height);
		case raw_image_format::rgb_interleaved: return import_rgb_interleaved(yuv_stream, width, height);
		default: throw std::invalid_argument("invalid raw image format");
	}
}


cv::Mat import_raw_mono(const std::string& yuv_filename, int width, int height, int bit_depth) {
	int type;
	switch(bit_depth) {
		case 8: type = CV_8U; break;
		case 16: type = CV_16U; break;
		default: throw std::invalid_argument("invalid raw image bit depth");
	}
	cv::Mat mat(height, width, type);
	int length = width * height * bit_depth/8;
	std::ifstream yuv_stream(yuv_filename, std::ios_base::binary);
	read_raw(yuv_stream, mat.data, length);
	return mat;
}


void export_raw_color(const cv::Mat& img, const std::string& yuv_filename, raw_image_format form) {
	std::ofstream yuv_stream(yuv_filename, std::ios_base::binary);
	switch(form) {
		case raw_image_format::ycbcr420: export_ycbcr420(yuv_stream, img); break;
		case raw_image_format::rgb_planar: export_rgb_planar(yuv_stream, img); break;
		case raw_image_format::rgb_interleaved: export_rgb_interleaved(yuv_stream, img); break;
		default: throw std::invalid_argument("invalid raw image format");
	}	
}


void export_raw_mono(const cv::Mat& img, const std::string& yuv_filename, int out_bit_depth) {
	int type;
	int in_bit_depth;
	switch(img.depth()) {
		case CV_8U: in_bit_depth = 8; break;
		case CV_16U: in_bit_depth = 16; break;
		default: throw std::invalid_argument("invalid input image bit depth");
	}
	switch(out_bit_depth) {
		case 8: type = CV_8U; break;
		case 16: type = CV_16U; break;
		default: throw std::invalid_argument("invalid raw image bit depth");
	}
	real alpha = std::exp2(out_bit_depth - in_bit_depth);
	cv::Mat out_mat;
	img.convertTo(out_mat, type, alpha);
	int length = img.cols * img.rows * out_bit_depth/8;
	std::ofstream yuv_stream(yuv_filename, std::ios_base::binary);
	write_raw(yuv_stream, out_mat.data, length);
}


}
