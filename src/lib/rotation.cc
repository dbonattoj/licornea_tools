#include "rotation.h"

namespace tlz {

constexpr real epsilon = 1.0e-6;

bool is_orthogonal_matrix(const mat33& R) {
	mat33 I(
		1.0, 0.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 0.0, 1.0
	);
	return cv::norm(I, R * R.t()) < epsilon;
}


mat33 to_rotation_matrix(const vec3& euler) {
	real x = euler[0], y = euler[1], z = euler[2];
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


vec3 to_euler(const mat33& R_) {
	if(! is_orthogonal_matrix(R_)) throw std::runtime_error("R is not an orthogonal matrix");
	
	mat33 R = R_.t();
	
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

	return vec3(x, y, z);
}


}
