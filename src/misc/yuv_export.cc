#include "../lib/common.h"
#include "../lib/args.h"
#include "../lib/raw_image_io.h"
#include "../lib/image_io.h"

using namespace tlz;


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "image.png out_image.yuv ycbcr420/rgb_planar/rgb_interleaved/mono8/mono16");
	std::string in_image_filename = in_filename_arg();
	std::string out_yuv_image_filename = out_filename_arg();
	std::string mode = enum_arg({ "ycbcr420", "rgb_planar", "rgb_interleaved", "mono8", "mono16" });

	if(mode == "ycbcr420") {
		cv::Mat_<cv::Vec3b> img = cv::imread(in_image_filename, cv::IMREAD_COLOR);
		export_raw_color(img, out_yuv_image_filename, raw_image_format::ycbcr420);
	} else if(mode == "rgb_planar") {
		cv::Mat_<cv::Vec3b> img = cv::imread(in_image_filename, cv::IMREAD_COLOR);
		export_raw_color(img, out_yuv_image_filename, raw_image_format::rgb_planar);
	} else if(mode == "rgb_interleaved") {
		cv::Mat_<cv::Vec3b> img = cv::imread(in_image_filename, cv::IMREAD_COLOR);
		export_raw_color(img, out_yuv_image_filename, raw_image_format::rgb_interleaved);
	} else if(mode == "mono8") {

	} else if(mode == "mono16") {

	} else {
		throw std::runtime_error("unknown yuv format");
	}
}
