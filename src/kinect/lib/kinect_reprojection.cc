#include "kinect_reprojection.h"
#include "common.h"


namespace tlz {

kinect_reprojection::kinect_reprojection(const kinect_reprojection_parameters& reproj) :
	reprojection_parameters_(reproj) { }


std::vector<vec2> kinect_reprojection::reproject_points_ir_to_color(
	const std::vector<vec2> distorted_ir_i_xy_points,
	const std::vector<real>& ir_z_points,
	std::vector<real>& out_color_z_points,
	bool distort_color
) {
	std::size_t n = distorted_ir_i_xy_points.size();
	assert(ir_z_points.size() == n);
	assert(out_color_z_point.size() == n);
	
	// undistort XY coordinates of ir points
	std::vector<vec2> undistorted_ir_i_xy_points(n);
	cv::undistortPoints(
		distorted_ir_i_xy_points,
		undistorted_ir_i_xy_points,
		reprojection_parameters_.ir_intrinsics.K,
		reprojection_parameters_.ir_intrinsics.distortion.cv_coeffs(),
		cv::noArray(),
		reprojection_parameters_.ir_intrinsics.K
	);
	
	// backproject to ir view space
	std::vector<vec3> color_v_points(n);
	#pragma omp parallel for
	for(int idx = 0; idx < n; ++idx) {
		const vec2& undistorted_ir_i_xy = undistorted_ir_i_xy_points[idx];
		const real& ir_z = ir_z_points[idx];		
		real& color_z = out_color_z_points[idx];
		vec3& color_v = color_v_points[idx];
		
		if(ir_z == 0.0) {
			color_z = 0.0;
			color_v = vec3(0.0, 0.0, 0.0);
			continue;
		}
		
		vec3 undistorted_ir_i_h(undistorted_ir_i_xy[0], undistorted_ir_i_xy[1], 1.0);
		undistorted_ir_i_h *= ir_z;
		
		vec3 ir_v = reprojection_parameters_.ir_intrinsics.K_inv * undistorted_ir_i_h;
		color_v = reprojection_parameters_.rotation.t() * (ir_v - reprojection_parameters_.translation);
		color_z = color_v[2];
	}
	
	// distort XY coordinates of color points, if requested
	std::vector<vec2> out_color_i_xy_points(n);
	cv::projectPoints(
		color_v_points,
		vec3::zeros(),
		vec3::zeros(),
		reprojection_parameters_.color_intrinsics.K,
		(distort_color ? reprojection_parameters_.color_intrinsics.distortion.cv_coeffs() : cv::noArray()),
		out_color_i_xy_points
	);
	
	return out_color_i_xy_points;
}


/*
vec3 kinect_reprojection::backproject_depth(vec2 undistorted, real z) const {
	const auto& ir_par = intrinsics_.ir;
	real x = ((undistorted[0] + 0.5f - ir_par.cx) / ir_par.fx) * z;
	real y = ((undistorted[1] + 0.5f - ir_par.cy) / ir_par.fy) * z;
	return vec3(x, y, z);
}
*/


}


