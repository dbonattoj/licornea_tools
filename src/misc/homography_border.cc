#include "../lib/common.h"
#include "../lib/args.h"
#include "../lib/json.h"
#include "../lib/opencv.h"
#include <vector>
#include <algorithm>

using namespace tlz;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "homography.json width height maximal_border/cropped_border out_border.json");
	mat33 homography = decode_mat(json_arg());
	int width = int_arg();
	int height = int_arg();
	std::string mode = enum_arg({ "maximal_border", "cropped_border" });
	std::string out_border_filename = out_filename_arg();
	
	std::vector<vec2> src {
		vec2(0, 0),
		vec2(width, 0),
		vec2(width, height),
		vec2(0, height)
	};
	
	std::vector<vec2> dst;
	cv::perspectiveTransform(src, dst, homography);
	
	int border_top = 0;
	int border_left = 0;
	int border_bottom = 0;
	int border_right = 0;
	if(mode == "maximal_border") {
		real max_x = std::max({ dst[0][0], dst[1][0], dst[2][0], dst[3][0] });
		real max_y = std::max({ dst[0][1], dst[1][1], dst[2][1], dst[3][1] });
		real min_x = std::min({ dst[0][0], dst[1][0], dst[2][0], dst[3][0] });
		real min_y = std::min({ dst[0][1], dst[1][1], dst[2][1], dst[3][1] });
		
		border_top = -min_y;
		border_left = -min_x;
		border_bottom = max_y - height;
		border_right = max_x - width;
		
	} else if(mode == "cropped_border") {
		// TODO
		
	}
	
	json j_out_border = json::object();
	j_out_border["top"] = border_top;
	j_out_border["left"] = border_left;
	j_out_border["bottom"] = border_bottom;
	j_out_border["right"] = border_right;
	export_json_file(j_out_border, out_border_filename);
}
