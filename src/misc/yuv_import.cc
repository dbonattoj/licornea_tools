#include "../lib/common.h"
#include "../lib/args.h"
#include "../lib/raw_image_io.h"
#include "../lib/image_io.h"

using namespace tlz;


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "image.yuv out_image.png width height ycbcr420/rgb_planar/rgb_interleaved/mono8/mono16");
	std::string in_yuv_image_filename = in_filename_arg();
	std::string out_image_filename = out_filename_arg();
	int width = int_arg();
	int height = int_arg();
	std::string mode = enum_arg({ "ycbcr420", "rgb_planar", "rgb_interleaved", "mono8", "mono16" });

	cv::Mat img;
	if(mode == "ycbcr420")
		img = import_raw_color(in_yuv_image_filename, width, height, raw_image_format::ycbcr420);
	else if(mode == "rgb_planar")
		img = import_raw_color(in_yuv_image_filename, width, height, raw_image_format::rgb_planar);
	else if(mode == "rgb_interleaved")
		img = import_raw_color(in_yuv_image_filename, width, height, raw_image_format::rgb_interleaved);
	else if(mode == "mono8")
		img = import_raw_mono(in_yuv_image_filename, width, height, 8);
	else if(mode == "mono16")
		img = import_raw_mono(in_yuv_image_filename, width, height, 16);
	else
		throw std::runtime_error("unknown yuv format");
		
	cv::imwrite(out_image_filename, img);
}
