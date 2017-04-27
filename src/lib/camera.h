#ifndef LICORNEA_CAMERA_H_
#define LICORNEA_CAMERA_H_

#include "../../lib/eigen.h"
#include <iosfwd>
#include <vector>
#include <map>
#include <string>

namespace tlz {

struct camera {
	std::string name;
	Eigen_mat3 intrinsic;
	Eigen_mat4 extrinsic;

	auto rotation() { return extrinsic.block<3, 3>(0, 0); }
	auto rotation() const { return extrinsic.block<3, 3>(0, 0); }
	auto translation() { return extrinsic.block<3, 1>(0, 3); }
	auto translation() const { return extrinsic.block<3, 1>(0, 3); }
};

using camera_array = std::vector<camera>;

camera_array read_cameras_file(const std::string& filename);
void write_cameras_file(const std::string& filename, const camera_array&);

std::map<std::string, camera> cameras_map(const std::vector<camera>&);
void write_cameras_file(const std::string& filename, const std::map<std::string, camera>&);

}

#endif
