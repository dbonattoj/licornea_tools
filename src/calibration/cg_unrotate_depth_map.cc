#include "../lib/args.h"
#include "../lib/json.h"
#include "../lib/image_io.h"
#include "../lib/intrinsics.h"
#include <string>

using namespace tlz;

int main(int args, const char* argv[]) {
	get_args(args, argv, "in_depth_map.png intr.json R.json out_depth_map.png");
	std::string in_depth_map_filename = in_filename_arg();
	intrinsics intr = intrinsics_arg();
	mat33 R = decode_mat(json_arg());
	std::string out_depth_map_filename = out_filename_arg();
	
	cv::Mat_<ushort> in_depth = load_depth(in_depth_map_filename);
	cv::Mat_<ushort> out_depth(in_depth.size());
	
	mat33 M = R.t() * intr.K_inv;
	#pragma omp parallel for
	for(int y = 0; y < in_depth.rows; ++y) for(int x = 0; x < in_depth.cols; ++x) {
		ushort in_d = in_depth(y, x);
		ushort out_d = 0;
		
		if(in_d != 0) {
			vec3 measured_i_h = in_d * vec3(x + 0.5, y + 0.5, 1.0);
			vec3 straight_i_h = M * measured_i_h;
			out_d = straight_i_h[2];
		}
		
		out_depth(y, x) = out_d;
	}
	
	save_depth(out_depth_map_filename, out_depth);
}
