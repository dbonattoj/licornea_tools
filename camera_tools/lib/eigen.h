#ifndef C3DLIC_EIGEN_H_
#define C3DLIC_EIGEN_H_

#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wdeprecated-register"
	#include <Eigen/Eigen>
#pragma GCC diagnostic pop

using Eigen_scalar = double;
using Eigen_mat3 = Eigen::Matrix3d;
using Eigen_mat4 = Eigen::Matrix4d;
using Eigen_vec3 = Eigen::Vector3d;

#endif
