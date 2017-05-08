#ifndef LICORNEA_CAMERA_H_
#define LICORNEA_CAMERA_H_

#include "opencv.h"
#include <iosfwd>
#include <vector>
#include <map>
#include <string>

namespace tlz {

struct camera {
	std::string name;
	mat33 intrinsic;
	mat33 rotation;
	vec3 translation;
	
	mat44 extrinsic() const;
};

using camera_array = std::vector<camera>;

camera_array read_cameras_file(const std::string& filename);
void write_cameras_file(const std::string& filename, const camera_array&);

std::map<std::string, camera> cameras_map(const std::vector<camera>&);
void write_cameras_file(const std::string& filename, const std::map<std::string, camera>&);

}

#endif
