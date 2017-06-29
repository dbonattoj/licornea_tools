#include "../lib/common.h"
#include "../lib/args.h"
#include <opencv2/opencv.hpp>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <fstream>
#include <stdexcept>

using namespace tlz;

void append_null_tail(std::ostream& output, std::streamsize length) {
	while(length--) output.put(0);
}


void export_yuv_disparity(std::string filename, const cv::Mat_<uchar>& mat) {
	std::ofstream output(filename, std::ios::binary);
	std::streamsize sz = mat.cols * mat.rows;
	output.write(reinterpret_cast<const std::ofstream::char_type*>(mat.data), sz);
	append_null_tail(output, sz);
}
void export_yuv_disparity(std::string filename, const cv::Mat_<ushort>& mat) {
	std::ofstream output(filename, std::ios::binary);
	std::streamsize sz = mat.cols * mat.rows * 2;
	output.write(reinterpret_cast<const std::ofstream::char_type*>(mat.data), sz);
	append_null_tail(output, sz);
}


template<typename Distance, typename Depth>
void orthogonal_distance_to_depth(
	const cv::Mat_<Distance>& in_z,
	cv::Mat_<Depth>& out_d,
	real d_near, real d_far,
	real z_near, real z_far
) {
	const real z_diff = z_far - z_near;
	const real offset = ((d_far * z_far) - (d_near * z_near)) / z_diff;
	const real factor = ((d_near - d_far) * z_near * z_far) / z_diff;	

	cv::Mat_<real> z_real = in_z;
	cv::Mat_<real> d_real = offset + factor / z_real;
	out_d = d_real;
}



int main(int argc, const char* argv[]) {
	get_args(argc, argv, "depth.png out_disparity.yuv z_near z_far [8/16]");
	std::string input_filename = in_filename_arg();
	std::string output_filename = out_filename_arg();
	ushort z_near = int_arg();
	ushort z_far = int_arg();
	bool output_disparity_16bit = (enum_opt_arg({"8", "16"}, "8") == "16");

	cv::Mat_<ushort> depth;
	{
		cv::Mat depth_ = cv::imread(input_filename, CV_LOAD_IMAGE_ANYDEPTH);
		if(depth_.depth() != CV_16U) throw std::runtime_error("input depth map: must be 16 bit");
		depth = depth_;	
	}

	if(output_disparity_16bit) {
		cv::Mat_<ushort> disparity;
		orthogonal_distance_to_depth<ushort, ushort>(depth, disparity, 0xffff, 0, z_near, z_far);
		disparity.setTo(0, depth == 0);

		export_yuv_disparity(output_filename, disparity);
	} else {
		cv::Mat_<uchar> disparity;
		orthogonal_distance_to_depth<ushort, uchar>(depth, disparity, 0xff, 0, z_near, z_far);
		disparity.setTo(0, depth == 0);

		export_yuv_disparity(output_filename, disparity);
	}

}
