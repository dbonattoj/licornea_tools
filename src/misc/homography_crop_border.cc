#include "../lib/common.h"
#include "../lib/args.h"
#include "../lib/view_homography.h"
#include "../lib/border.h"
#include "../lib/opencv.h"
#include <string>
#include <functional>
#include <vector>

using namespace tlz;

const cv::Vec3b black(0, 0, 0);
const cv::Vec3b white(255, 255, 255);
const std::string window_name = "Crop Border";

std::function<void()> update_function;
void update_callback(int = 0, void* = nullptr) {	
	update_function();
}

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "homography.json/homographies.json width height out_crop_border.json [unit=1]");
	json j = json_arg();
	int width = int_arg();
	int height = int_arg();
	std::string out_border_filename = out_filename_arg();
	int unit = int_opt_arg(1);
	
	std::vector<std::vector<vec2>> quadrilaterals;
	
	border max_bord;
	if(is_single_view_homography(j)) {
		view_homography	hom = decode_view_homography(j);
		max_bord = maximal_border(hom, width, height);
		quadrilaterals.push_back(quadrilateral(hom, width, height));
	} else {
		view_homographies homs = decode_view_homographies(j);
		max_bord = maximal_border(homs, width, height);
		for(const auto& kv : homs)
			quadrilaterals.push_back(quadrilateral(kv.second, width, height));
	}
	
	int out_x = max_bord.left;
	int out_y = max_bord.top;
	int out_width = (width / unit) * unit;
	int out_height = (height / unit) * unit;
	
	cv::Size size = add_border(max_bord, cv::Size(width, height));
	cv::Mat_<cv::Vec3b> back_img(size);
	back_img.setTo(black);
	for(const auto& quad : quadrilaterals) {
		std::vector<cv::Point> pts;
		for(const vec2& pt : quad)
			pts.emplace_back(pt[0] + max_bord.left, pt[1] + max_bord.top);
		pts.push_back(pts.front());
		cv::fillConvexPoly(back_img, pts.data(), 4, cv::Scalar(white));
	}

	cv::Mat_<cv::Vec3b> shown_img(size);
	auto update = [&]() {
		back_img.copyTo(shown_img);
		cv::imshow(window_name, shown_img);
	};
	update_function = update;

	cv::namedWindow(window_name, CV_WINDOW_NORMAL);

	cv::createTrackbar("x", window_name, &out_x, size.width, &update_callback);
	cv::createTrackbar("y", window_name, &out_y, size.height, &update_callback);
	cv::createTrackbar("width", window_name, &out_x, size.width, &update_callback);
	cv::createTrackbar("height", window_name, &out_y, size.height, &update_callback);

	update();

	while(cv::waitKey(0) != escape_keycode);
}
