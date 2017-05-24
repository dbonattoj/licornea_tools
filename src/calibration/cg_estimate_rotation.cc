#include <string>
#include <vector>
#include <cmath>
#include <iostream>
#include "../lib/utility/misc.h"
#include "../lib/json.h"
#include "../lib/opencv.h"
#include "../lib/intrinsics.h"

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: cg_estimate_rotation slopes.json intrinsics.json out_rotation.json\n";
	std::cout << std::endl;
	std::exit(1);
}

constexpr real pi = 3.14159265359;
constexpr real deg_per_rad = 180.0 / pi;
constexpr real rad_per_deg = pi / 180.0;


mat33 to_rotation_matrix(const vec3 vec) {
	real x = vec[0], y = vec[1], z = vec[2];
	mat33 Rx(
		1.0, 0.0, 0.0,
		0.0, std::cos(x), -std::sin(x),
		0.0, std::sin(x), std::cos(x)
	);
	mat33 Ry(
		std::cos(y), 0.0, std::sin(y),
		0.0, 1.0, 0.0,
		-std::sin(y), 0.0, std::cos(y)
	);
	mat33 Rz(
		std::cos(z), -std::sin(z), 0.0,
		std::sin(z), std::cos(z), 0.0,
		0.0, 0.0, 1.0
	);
	return Rx * Ry * Rz;
}


int main(int argc, const char* argv[]) {
	if(argc <= 3) usage_fail();
	std::string slopes_filename = argv[1];
	std::string intrinsics_filename = argv[2];
	std::string out_rotation_filename = argv[3];
	
	std::cout << "loading slopes" << std::endl;
	json j_slopes = import_json_file(slopes_filename);

	struct feature_slope {
		vec2 point;
		real horizontal;
		real vertical;
	};
	std::vector<feature_slope> feature_slopes;
	std::vector<vec2> distorted_coordinates;

	{
		json j_feature_slopes = j_slopes["slopes"];
		for(auto it = j_feature_slopes.begin(); it != j_feature_slopes.end(); ++it) {
			json j_feature_slope = it.value();
			feature_slope fslope;
			fslope.horizontal = j_feature_slope["horizontal"];
			fslope.vertical = j_feature_slope["vertical"];
			feature_slopes.push_back(fslope);
			distorted_coordinates.emplace_back(j_feature_slope["ix"],j_feature_slope["iy"]);
		}
	}
	
	std::cout << "loading intrinsics" << std::endl;
	intrinsics intr = decode_intrinsics(import_json_file(intrinsics_filename));
	
	std::cout << "undistorting coordinates" << std::endl;
	{
		std::vector<vec2> undistorted_coordinates = undistort_points(intr, distorted_coordinates);
		for(std::ptrdiff_t idx = 0; idx < undistorted_coordinates.size(); ++idx)
			feature_slopes[idx].point = undistorted_coordinates[idx];
	}
	
	auto rotation_error = [&feature_slopes, &intr](const mat33& R) {
		real fx = intr.fx(), fy = intr.fy(), cx = intr.cx(), cy = intr.cy();
		real r11 = R(0, 0), r21 = R(1, 0), r31 = R(2, 0);
		real r12 = R(0, 1), r22 = R(1, 1), r32 = R(2, 1);
		
		real err_sum = 0.0;
		for(const feature_slope& fslope : feature_slopes) {
			real ix = fslope.point[0], iy = fslope.point[1];
			real model_hslope = (fy*r21 + cy*r31 - iy*r31) / (fx*r11 + cx*r31 - ix*r31);
			real model_vslope = (fx*r12 + cx*r32 - ix*r32) / (fy*r22 + cy*r32 - iy*r32);
			
			real data_hslope = fslope.horizontal;
			real data_vslope = fslope.vertical;
			
			real err = sq(model_hslope - data_hslope) + sq(model_vslope - data_vslope);
			err_sum += err;
		}
		err_sum /= feature_slopes.size();
		return err_sum;
	};
	
	std::cout << "searching rotations space" << std::endl;
	real x_min = -10.0 * rad_per_deg;
	real x_max = +10.0 * rad_per_deg;
	real x_step = 0.1 * rad_per_deg;

	real y_min = -10.0 * rad_per_deg;
	real y_max = +10.0 * rad_per_deg;
	real y_step = 0.1 * rad_per_deg;

	real z_min = -10.0 * rad_per_deg;
	real z_max = +10.0 * rad_per_deg;
	real z_step = 0.1 * rad_per_deg;
	
	mat33 min_err_R;
	vec3 min_err_R_vec;
	real min_err = INFINITY;
	for(real x = x_min; x <= x_max; x += x_step)
	for(real y = y_min; y <= y_max; y += y_step)
	for(real z = z_min; z <= z_max; z += z_step) {
		vec3 R_vec(x, y, z);
		mat33 R = to_rotation_matrix(R_vec);
		real err = rotation_error(R);
		if(err < min_err) {
			min_err = err;
			min_err_R = R;
			min_err_R_vec = R_vec;
		}
		std::cout << '.' << std::flush;
	}
	
	std::cout << "\n" << std::endl;
	std::cout << "x = " << min_err_R_vec[0] * deg_per_rad << std::endl;
	std::cout << "y = " << min_err_R_vec[1] * deg_per_rad << std::endl;
	std::cout << "z = " << min_err_R_vec[2] * deg_per_rad << std::endl;
}
