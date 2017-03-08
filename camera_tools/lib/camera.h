#ifndef C3DLIC_CAMERA_H_
#define C3DLIC_CAMERA_H_

#include "eigen.h"
#include <iosfwd>
#include <vector>
#include <string>

struct camera {
	std::string name;
	Eigen_mat3 intrinsic;
	Eigen_mat4 extrinsic;

	auto rotation() { return extrinsic.block<3, 3>(0, 0); }
	auto rotation() const { return extrinsic.block<3, 3>(0, 0); }
	auto translation() { return extrinsic.block<3, 1>(0, 3); }
	auto translation() const { return extrinsic.block<3, 1>(0, 3); }
};

template<typename It> void read_cameras(std::istream& input, It output);
template<typename It> void write_cameras(std::ostream& output, It begin, It end);

std::vector<camera> read_cameras_file(const std::string& filename);
void write_cameras_file(const std::string& filename, const std::vector<camera>&);

#include "camera.tcc"

#endif