#include "../lib/common.h"
#include "../lib/args.h"
#include "../lib/image_io.h"
#include "../lib/opencv.h"
#include "../lib/json.h"

using namespace tlz;

int main(int argc, const char* argv[]) {
	auto args = get_args(argc, argv,
		"homography.json in_image.png out_image.png texture/depth");
	std::string homography_filename = args.in_filename_arg();
	std::string in_image_filename = args.in_filename_arg();
	std::string out_image_filename = args.out_filename_arg();
	std::string image_type = args.enum_arg({ "texture", "depth" });
	
	mat33 homography = decode_mat(import_json_file(homography_filename));
	homography = homography.inv();
	
	if(image_type == "texture") {
		cv::Mat_<cv::Vec3b> in_image = load_texture(in_image_filename);
		cv::Mat_<cv::Vec3b> out_image;
		cv::Size dsize = in_image.size();
		cv::warpPerspective(in_image, out_image, homography, dsize, cv::INTER_LINEAR);
		save_texture(out_image_filename, out_image);
		
	} else if(image_type == "depth") {
		
	}
}
