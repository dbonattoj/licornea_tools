#include "../lib/common.h"
#include "../lib/args.h"
#include "../lib/image_io.h"
#include "../lib/opencv.h"
#include "../lib/json.h"

using namespace tlz;

int main(int argc, const char* argv[]) {
	get_args(argc, argv,
		"homography.json in_image.png out_image.png texture/depth border_top border_left border_bottom border_right [invert]");
	std::string homography_filename = in_filename_arg();
	std::string in_image_filename = in_filename_arg();
	std::string out_image_filename = out_filename_arg();
	std::string image_type = enum_arg({ "texture", "depth" });	
	int border_top = int_arg();
	int border_left = int_arg();
	int border_bottom = int_arg();
	int border_right = int_arg();
	bool invert = bool_opt_arg("invert");
	
	mat33 offset(
		1, 0, border_left,
		0, 1, border_top,
		0, 0, 1
	);
	
	mat33 homography = decode_mat(import_json_file(homography_filename));
	if(invert) homography = homography.inv();
	homography = offset * homography;
	
	if(image_type == "texture") {
		cv::Mat_<cv::Vec3b> in_image = load_texture(in_image_filename);
		cv::Mat_<cv::Vec3b> out_image;
		cv::Size dsize = in_image.size();
		dsize.width += (border_left + border_right);
		dsize.height += (border_top + border_bottom);
		cv::warpPerspective(in_image, out_image, homography, dsize, cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(cv::Vec3b(255,255,255)));
		save_texture(out_image_filename, out_image);
		
	} else if(image_type == "depth") {
		
	}
}
