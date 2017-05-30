#include <cassert>

namespace tlz {

template<typename Value, typename Depth>
std::vector<kinect_remapping::sample<Value>> kinect_remapping::remap_ir_to_color_samples(
	const cv::Mat_<Value>& distorted_ir_values_img,
	const cv::Mat_<Depth>& distorted_ir_z_img
) const {
	assert(distorted_ir_values.cols == 512 && distorted_ir_values.rows == 424);
	std::vector<vec2> distorted_ir_i_xy_points;
	std::vector<real> ir_i_z_points;

	distorted_ir_i_xy_points.reserve(512*424);
	ir_i_z_points.reserve(512*424);
	
	std::vector<sample<Value>> samples;
	for(int ir_y = 0; ir_y < 424; ++ir_y) for(int ir_x = 0; ir_x < 512; ++ir_x) {
		Depth ir_z = distorted_ir_z_img(ir_y, ir_x);
		if(ir_z <= 0.001) continue;

		vec2 dist_ir(ir_x + 0.5, ir_y + 0.5);
		vec2 undist_ir = undistort_ir(dist_ir);
		vec2 color = map_ir_to_color(undist_ir, ir_z);

		sample<Value> samp;
		samp.ir_coordinates = vec2(ir_x, ir_y);
		samp.color_depth = samp.ir_depth = ir_z;
		samp.color_coordinates = color;
		
		samples.push_back(samp);
	}
	
	return samples;
}

}
