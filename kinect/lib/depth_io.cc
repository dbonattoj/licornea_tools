#include "depth_io.h"
#include "common.h"

namespace tlz {

cv::Mat_<ushort> load_depth(const char* filename, bool any_size) {
	cv::Mat mat = cv::imread(filename, CV_LOAD_IMAGE_ANYDEPTH);
	if(mat.depth() != CV_16U) throw std::runtime_error("input depth map: must be 16 bit");
	if(! any_size)
		if(mat.rows != depth_height || mat.cols != depth_width) throw std::runtime_error("input depth map: wrong size");
	cv::Mat_<ushort> mat_ = mat;
	return mat_;
}


void save_depth(const char* filename, const cv::Mat_<ushort>& depth) {
	std::vector<int> params = { CV_IMWRITE_PNG_COMPRESSION, 0 };
	cv::imwrite(filename, depth, params);
}

}

