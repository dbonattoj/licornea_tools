#include <string>
#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>
#include <nelder-mead-optimizer/optimizer.h>
#include "../lib/utility/misc.h"
#include "../lib/json.h"
#include "../lib/opencv.h"
#include "../lib/intrinsics.h"

using namespace tlz;


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


[[noreturn]] void usage_fail() {
	std::cout << "usage: cg_estimate_rotation slopes.json intrinsics.json out_rotation.json" << std::endl;
	std::exit(EXIT_FAILURE);
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
	vec3 rotation_vec;
	{
		using namespace blinry;
		const real precision = 0.00001;
		NelderMeadOptimizer optimizer(3, precision);
		Vector v(0.0, 0.0, 0.0);
		vec3 step = vec3(1, 1, 1) * rad_per_deg;
		optimizer.insert(v);
		optimizer.insert(Vector(v[0] + step[0], v[1], v[2]));
		optimizer.insert(Vector(v[0], v[1] + step[1], v[2]));
		optimizer.insert(Vector(v[0], v[1], v[2] +  + step[2]));
		auto f = [&rotation_error](Vector& vec) {
			mat33 R = to_rotation_matrix(vec3(vec[0], vec[1], vec[2]));
			return rotation_error(R);
		};
		while(! optimizer.done()) {
			rotation_vec = vec3(v[0], v[1], v[2]);
			std::cout << '.' << std::flush;
			v = optimizer.step(v, f(v));
		}
		std::cout << std::endl;
	}
	
	std::cout << "x = " << rotation_vec[0] * deg_per_rad << std::endl;
	std::cout << "y = " << rotation_vec[1] * deg_per_rad << std::endl;
	std::cout << "z = " << rotation_vec[2] * deg_per_rad << std::endl;
	
}
