#ifndef LICORNEA_CAMERA_H_
#define LICORNEA_CAMERA_H_

#include "common.h"
#include "json.h"
#include "../../lib/intrinsics.h"
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
	mat44 extrinsic_inv() const;
	
	vec3 position() const;
};

using camera_array = std::vector<camera>;

camera_array decode_cameras(const json&);
void export_cameras_file(const camera_array& cams, const std::string& filename);

camera_array cameras_arg();
intrinsics to_undistorted_intrinsics(const camera&, int width, int height);

std::map<std::string, camera> cameras_map(const std::vector<camera>&);
void export_cameras_file(const std::string& filename, const std::map<std::string, camera>&);

}

#endif
