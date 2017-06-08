#include "../lib/args.h"
#include "../lib/json.h"
#include <iostream>
#include <cstdlib>
#include <cmath>

using namespace tlz;

constexpr real epsilon = 1.0e-6;

bool is_orthogonal_matrix(const mat33& R) {
	mat33 I(
		1.0, 0.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 0.0, 1.0
	);
	return cv::norm(I, R * R.t()) < epsilon;
}

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "(from rotation_matrix.json / to out_rotation_matrix.json x y z)");
	std::string mode = enum_arg({"from", "to"});
	
	if(mode == "from") {
		std::string rotation_mat_filename = in_filename_arg();
				
		mat33 R = decode_mat(import_json_file(rotation_mat_filename));
		if(! is_orthogonal_matrix(R)) throw std::runtime_error("R is not an orthogonal matrix");
		
		R = R.t();
		
		// https://www.learnopencv.com/rotation-matrix-to-euler-angles/
		
		real sy = std::sqrt(R(0,0)*R(0,0) + R(1,0)*R(1,0));
		bool singular = (sy < epsilon);
		real x, y, z;
		
		if(! singular) {
			x = std::atan2(R(2,1), R(2,2));
			y = std::atan2(-R(2,0), sy);
			z = std::atan2(R(1,0), R(0,0));
		} else {
			x = std::atan2(-R(1,2), R(1,1));
			y = std::atan2(-R(2,0), sy);
			z = 0.0;
		}
		
		std::cout << "x = " << x * deg_per_rad << "°" << std::endl;
		std::cout << "y = " << y * deg_per_rad << "°" << std::endl;
		std::cout << "z = " << z * deg_per_rad << "°" << std::endl;
		
	} else if(mode == "to") {
		std::string out_rotation_mat_filename = out_filename_arg();
		real x = real_arg();
		real y = real_arg();
		real z = real_arg();
		
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
		R = R.t();
		
		export_json_file(encode_mat(R), out_rotation_mat_filename);
	}	
}
