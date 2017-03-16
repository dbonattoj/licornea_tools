#include "camera.h"
#include <json.hpp>

namespace tlz {

template<typename It>
void read_cameras(std::istream& input, It output) {
	using namespace nlohmann;
	
	json j_root;
	input >> j_root;
		
	for(auto& j_cam : j_root) {
		camera cam;
		cam.name = j_cam["name"];
		cam.intrinsic <<
			j_cam["K"][0][0], j_cam["K"][0][1], j_cam["K"][0][2],
			j_cam["K"][1][0], j_cam["K"][1][1], j_cam["K"][1][2],
			j_cam["K"][2][0], j_cam["K"][2][1], j_cam["K"][2][2];
		cam.extrinsic <<
			j_cam["Rt"][0][0], j_cam["Rt"][0][1], j_cam["Rt"][0][2], j_cam["Rt"][0][3],
			j_cam["Rt"][1][0], j_cam["Rt"][1][1], j_cam["Rt"][1][2], j_cam["Rt"][1][3],
			j_cam["Rt"][2][0], j_cam["Rt"][2][1], j_cam["Rt"][2][2], j_cam["Rt"][2][3],
			j_cam["Rt"][3][0], j_cam["Rt"][3][1], j_cam["Rt"][3][2], j_cam["Rt"][3][3];
		
		*output = cam;
	}
}


template<typename It>
void write_cameras(std::ostream& output, It begin, It end) {
	using namespace nlohmann;
	
	json j_root = json::array();
	
	for(It it = begin; it != end; ++it) {
		const camera& cam = *it;
		json j_cam = json::object();
		j_cam["name"] = cam.name;
		j_cam["K"] = {
			{ cam.intrinsic(0, 0), cam.intrinsic(0, 1), cam.intrinsic(0, 2) },
			{ cam.intrinsic(1, 0), cam.intrinsic(1, 1), cam.intrinsic(1, 2) },
			{ cam.intrinsic(2, 0), cam.intrinsic(2, 1), cam.intrinsic(2, 2) }
		};
		j_cam["Rt"] = {
			{ cam.extrinsic(0, 0), cam.extrinsic(0, 1), cam.extrinsic(0, 2), cam.extrinsic(0, 3) },
			{ cam.extrinsic(1, 0), cam.extrinsic(1, 1), cam.extrinsic(1, 2), cam.extrinsic(1, 3) },
			{ cam.extrinsic(2, 0), cam.extrinsic(2, 1), cam.extrinsic(2, 2), cam.extrinsic(2, 3) },
			{ cam.extrinsic(3, 0), cam.extrinsic(3, 1), cam.extrinsic(3, 2), cam.extrinsic(3, 3) }
		};
		j_root.push_back(j_cam);
	}
	
	j_root >> output;
}

}
