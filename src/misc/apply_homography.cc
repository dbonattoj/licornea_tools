#include "../lib/common.h"
#include "../lib/args.h"
#include "../lib/image_io.h"
#include "../lib/opencv.h"
#include "../lib/json.h"
#include "../lib/border.h"

using namespace tlz;

int main(int argc, const char* argv[]) {
	get_args(argc, argv,
		"homography.json in_image.png out_image.png texture/depth [border.json]");
	std::string homography_filename = in_filename_arg();
	std::string in_image_filename = in_filename_arg();
	std::string out_image_filename = out_filename_arg();
	std::string image_type = enum_arg({ "texture", "depth" });
	std::string border_filename = in_filename_opt_arg();
	border bord;
	if(! border_filename.empty()) bord = decode_border(import_json_file(border_filename));
			
	mat33 offset(
		1, 0, bord.left,
		0, 1, bord.top,
		0, 0, 1
	);
	
	mat33 homography = decode_mat(import_json_file(homography_filename));

	homography = offset * homography;
	
	if(image_type == "texture") {
		cv::Mat_<cv::Vec3b> in_image = load_texture(in_image_filename);
		cv::Mat_<cv::Vec3b> out_image;
		cv::Size dsize = add_border(bord, in_image.size());
		cv::warpPerspective(in_image, out_image, homography, dsize, cv::INTER_CUBIC, cv::BORDER_CONSTANT, cv::Scalar(cv::Vec3b(255,255,255)));
		save_texture(out_image_filename, out_image);
		
	} else if(image_type == "depth") {
		cv::Mat_<ushort> in_image = load_depth(in_image_filename);
		cv::Mat_<ushort> out_image;
		cv::Size dsize = add_border(bord, in_image.size());
		cv::warpPerspective(in_image, out_image, homography, dsize, cv::INTER_NEAREST, cv::BORDER_CONSTANT, cv::Scalar(cv::Vec3b(255,255,255)));
		save_depth(out_image_filename, out_image);

	}
}
