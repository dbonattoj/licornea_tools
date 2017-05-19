#include <cassert>

namespace tlz {

template<typename T>
std::vector<kinect_reprojection::sample<T>> kinect_reprojection::reproject_ir_to_color_samples(
	const cv::Mat_<T>& distorted_ir_values_img,
	const cv::Mat_<real>& distorted_ir_z_img,
	bool distort_color
) {
	assert(distorted_ir_values.cols == 512 && distorted_ir_values.rows == 424);
	std::vector<vec2> distorted_ir_i_xy_points;
	std::vector<real> ir_i_z_points;

	distorted_ir_i_xy_points.reserve(512*424);
	ir_i_z_points.reserve(512*424);
	
	for(int ir_y = 0; ir_y < 424; ++ir_y) for(int ir_x = 0; ir_x < 512; ++ir_x) {
		real ir_z = distorted_ir_z_img(ir_y, ir_x);
		if(ir_z <= 0.001) continue;
		distorted_ir_i_xy_points.emplace_back(ir_x, ir_y);
		ir_i_z_points.emplace_back(ir_z);
	}
	
	std::size_t n = distorted_ir_i_xy_points.size();

	std::vector<real> color_i_z_points(n);
	std::vector<vec2> color_i_xy_points = reproject_points_ir_to_color(
		distorted_ir_i_xy_points,
		ir_i_z_points,
		color_i_z_points,
		distort_color
	);
	
	std::vector<sample<T>> samples(n);
	#pragma omp parallel for
	for(std::ptrdiff_t idx = 0; idx < n; ++idx) {
		sample<T>& samp = samples[idx];
		samp.color_coordinates = color_i_xy_points[idx];
		samp.ir_coordinates = distorted_ir_i_xy_points[idx];
		samp.color_depth = color_i_z_points[idx];
		samp.ir_depth = ir_i_z_points[idx];
		int ir_x = samp.ir_coordinates[0], ir_y = samp.ir_coordinates[1];
		samp.value = distorted_ir_values_img(ir_y, ir_x);
	}
	return samples;
}

}
