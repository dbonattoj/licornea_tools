#ifndef C3DLIC_CAMERA_H_
#define C3DLIC_CAMERA_H_

#include "eigen.h"
#include <iosfwd>

struct camera {
	std::string name;
	Eigen_mat3 intrinsic;
	Eigen_mat3 rotation;
	Eigen_vec3 translation;
};

bool read_camera(std::istream& input, camera&);
void write_camera(std::ostream& output, const camera&);

#endif
