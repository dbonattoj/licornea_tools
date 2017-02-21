#include "texture_io.h"

cv::Mat_<cv::Vec3b> load_texture(const char* filename) {
	cv::Mat mat = cv::imread(filename, CV_LOAD_IMAGE_COLOR);
	if(mat.rows != texture_height || mat.cols != texture_width) throw std::runtime_error("input texture: wrong size");
	cv::Mat_<cv::Vec3b> mat_ = mat;
	return mat_;
}


void save_texture(const char* filename, const cv::Mat_<cv::Vec3b>& texture) {
	std::vector<int> params = { CV_IMWRITE_PNG_COMPRESSION, 0 };
	cv::imwrite(filename, texture, params);
}
