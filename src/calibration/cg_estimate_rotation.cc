#include <string> 
#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>
#include <cassert>
#include "lib/cg/feature_slopes.h"
#include "../lib/misc.h"
#include "../lib/json.h"
#include "../lib/opencv.h"
#include "../lib/intrinsics.h"
#include "../lib/assert.h"

using namespace tlz;


constexpr real pi = 3.14159265359;
constexpr real deg_per_rad = 180.0 / pi;
constexpr real rad_per_deg = pi / 180.0;

constexpr real gr = 1.61803398875;

constexpr bool verbose = true;


mat33 to_rotation_matrix(real x, real y, real z) {
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
	mat33 R = Rz * Ry * Rx;
	return R.t();
}


template<typename Func>
void one_dim_search_minimum(Func f, real& a, real& b, real tolerance) {
	assert(a < b);
	
	real c = b - (b - a)/gr;
	real d = a + (b - a)/gr;
	
	while(std::abs(c - d) > tolerance) {
		if(f(c) < f(d)) b = d;
		else a = c;
        c = b - (b - a) / gr;
        d = a + (b - a) / gr;
	}
}


[[noreturn]] void usage_fail() {
	std::cout << "usage: cg_estimate_rotation measured_feature_slopes.json intrinsics.json out_rotation.json" << std::endl;
	std::exit(EXIT_FAILURE);
}
int main(int argc, const char* argv[]) {
	if(argc <= 3) usage_fail();
	std::string measured_feature_slopes_filename = argv[1];
	std::string intrinsics_filename = argv[2];
	std::string out_rotation_filename = argv[3];
	
	std::cout << "loading intrinsics" << std::endl;
	intrinsics intr = decode_intrinsics(import_json_file(intrinsics_filename));

	std::cout << "loading slopes" << std::endl;
	feature_slopes measured_fslopes = decode_feature_slopes(import_json_file(measured_feature_slopes_filename));
	Assert(! measured_fslopes.is_distorted);
	
	auto rotation_error = [&measured_fslopes, &intr](real x, real y, real z) {
		mat33 R = to_rotation_matrix(x, y, z);
		
		real fx = intr.fx(), fy = intr.fy(), cx = intr.cx(), cy = intr.cy();
		real r11 = R(0, 0), r21 = R(1, 0), r31 = R(2, 0);
		real r12 = R(0, 1), r22 = R(1, 1), r32 = R(2, 1);
		
		real err_sum = 0.0;
		for(const auto& kv : measured_fslopes.slopes) {
			const std::string& feature_name = kv.first;
			const vec2& undist_point = measured_fslopes.points.at(feature_name);
			const feature_slope& measured_fslope = kv.second;
			
			real ix = undist_point[0], iy = undist_point[1];
			real model_hslope = (fy*r21 + cy*r31 - iy*r31) / (fx*r11 + cx*r31 - ix*r31);
			real model_vslope = (fx*r12 + cx*r32 - ix*r32) / (fy*r22 + cy*r32 - iy*r32);
			
			real measured_hslope = measured_fslope.horizontal;
			real measured_vslope = measured_fslope.vertical;
			
			real err = sq(model_hslope - measured_hslope) + sq(model_vslope - measured_vslope);
			err_sum += err;
		}
		err_sum /= measured_fslopes.slopes.size();
		return err_sum;
	};
	
	std::cout << "minimizing rotations error" << std::endl;
	real z = 0.0, x = 0.0, y = 0.0;
	{		
		auto f_x = [&](real x_) { return rotation_error(x_, y, z); };
		auto f_y = [&](real y_) { return rotation_error(x, y_, z); };
		auto f_z = [&](real z_) { return rotation_error(x, y, z_); };
		
		real initial_outreach = 10.0 * rad_per_deg;
		real initial_tolerance = 1.0 * rad_per_deg;
		real min_error = 1e-10;
		real min_error_diff = 1e-10;
		int max_iterations = -1;
		real outreach_scaledown = 0.001;
		real tolerance_scaledown = 0.005;
		
		int iterations = 0;
		real outreach = initial_outreach;
		real tolerance = initial_tolerance;
		real err = rotation_error(x, y, z);
		while(err > min_error && iterations != max_iterations) {
			if(verbose) {
				std::cout << "iterations = " << iterations << "\n";
				std::cout << "err = " << err << "\n";
				std::cout << "x = " << x * deg_per_rad << "\n";
				std::cout << "y = " << y * deg_per_rad << "\n";
				std::cout << "z = " << z * deg_per_rad << "\n";
				std::cout << "outreach = " << outreach << "\n";
				std::cout << "tolerance = " << tolerance << "\n\n" << std::endl;
			} else {
				std::cout << '.' << std::flush;
			}
			
			real z_min = z - outreach, z_max = z + outreach;
			one_dim_search_minimum(f_z, z_min, z_max, tolerance);
			z = (z_max + z_min) / 2.0;
		
			real x_min = x - outreach, x_max = x + outreach;
			one_dim_search_minimum(f_x, x_min, x_max, tolerance);
			x = (x_max + x_min) / 2.0;

			real y_min = y - outreach, y_max = y + outreach;
			one_dim_search_minimum(f_y, y_min, y_max, tolerance);
			y = (y_max + y_min) / 2.0;
					
			++iterations;
			outreach = initial_outreach / std::exp(iterations * outreach_scaledown);
			tolerance = initial_tolerance / std::exp(iterations * tolerance_scaledown);
			real prev_err = err;
			err = rotation_error(x, y, z);
			
			if(std::abs(prev_err - err) < min_error_diff) break; 
		}
		
		std::cout << "\nfound minimum err = " << err << "\n";
		std::cout << "x = " << x * deg_per_rad << "\n";
		std::cout << "y = " << y * deg_per_rad << "\n";
		std::cout << "z = " << z * deg_per_rad << std::endl;
	}
	
	std::cout << "saving rotation matrix" << std::endl;
	mat33 R = to_rotation_matrix(x, y, z);
	export_json_file(encode_mat(R), out_rotation_filename);
}
