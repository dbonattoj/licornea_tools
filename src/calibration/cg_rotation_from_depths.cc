#include <string> 
#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>
#include <cassert>
#include "lib/image_correspondence.h"
#include "lib/feature_point.h"
#include "../lib/args.h"
#include "../lib/rotation.h"
#include "../lib/misc.h"
#include "../lib/json.h"
#include "../lib/intrinsics.h"
#include "../lib/assert.h"
#include "../lib/opencv.h"
#include "../lib/eigen.h"

using namespace tlz;

constexpr bool verbose = true;

// rotation which puts normal onto (0,0,1)
using xy_rotation = vec3;

xy_rotation estimate_xy_rotation(const mat33& K_inv, const image_correspondence_feature& feature) {
	// get points v in camera's view spaces
	std::vector<Eigen_vec3> v_points;
	v_points.reserve(feature.points.size());
	Eigen_vec3 v_mean(0.0, 0.0, 0.0);
	for(const auto& kv : feature.points) {
		const feature_point& fpoint = kv.second;
		if(fpoint.depth == 0.0) continue;
		
		Eigen_vec3 i_h = Eigen_vec3(fpoint.position[0], fpoint.position[1], 1.0) * fpoint.depth;
		Eigen_vec3 v = to_eigen(K_inv) * i_h;
		v_points.push_back(v);
		v_mean += v;
	}
	std::size_t n = v_points.size();
	if(n < 100) return vec3(0.0,0.0);
	
	v_mean = v_mean / real(n);
	
	// fit plane to these points
	Eigen_matnX<3> A(3, n);
	for(std::ptrdiff_t i = 0; i < n; ++i) {
		const Eigen_vec3& v = v_points[i];
		A(0, i) = v[0] - v_mean[0];
		A(1, i) = v[1] - v_mean[1];
		A(2, i) = v[2] - v_mean[2];
	}

	Eigen::JacobiSVD<decltype(A)> svd(A, Eigen::ComputeThinU);
	auto U = svd.matrixU();
	Eigen_vec3 normal(U(0,2), U(1,2), U(2,2));
	if(normal[2] < 0.0) normal = -normal;
	
	return from_eigen(normal);
}


mat33 xy_rotation_matrix(const vec3& normal) {
	vec3 z(0.0, 0.0, 1.0);
	vec3 v = normal.cross(z);
	mat33 v_mat(
		0.0, -v[2], v[1],
		v[2], 0.0, -v[0],
		-v[1], v[0], 0.0
	);
	real c = normal.dot(z);
	real f = 1.0 / (1.0 + c);
	mat33 R = mat33::eye() + v_mat + f*v_mat*v_mat;
	return R;
}




int main(int argc, const char* argv[]) {
	get_args(argc, argv, "cors.json intrinsics.json out_rotation.json");
	image_correspondences cors = image_correspondences_arg();
	intrinsics intr = intrinsics_arg();
	std::string out_rotation_filename = out_filename_arg();
	
	vec3 xy_normal_sum = 0.0;
	for(const auto& kv : cors.features) {
		const image_correspondence_feature& feature = kv.second;
		xy_rotation xy = estimate_xy_rotation(intr.K_inv, feature);
		xy_normal_sum += xy;
	}
	vec3 xy_normal = xy_normal_sum / cv::norm(xy_normal_sum);

	std::cout << sq(xy_normal[0])+sq(xy_normal[1])+sq(xy_normal[2]) << std::endl;
	
	mat33 R = xy_rotation_matrix(xy_normal);
	std::cout << R << std::endl;
	vec3 euler = to_euler(R);
	std::cout << "x = " << euler[0] * deg_per_rad << "\n";
	std::cout << "y = " << euler[1] * deg_per_rad << "\n";
	std::cout << "z = " << euler[2] * deg_per_rad << std::endl;
}
