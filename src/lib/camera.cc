#include "camera.h"
#include "json.h"
#include <fstream>
#include <iomanip>
#include <iterator>

namespace tlz {
	
namespace {

struct frm {
	real value_;
	explicit frm(real value) : value_(value) { }
}; 

std::ostream& operator<<(std::ostream& stream, const frm& f) {
	stream << std::setprecision(4) << std::setw(7) << std::setfill(' ') << f.value_;
	return stream;
}

}
	
camera_array read_cameras_file(const std::string& filename) {
	camera_array cameras;
	
	using namespace nlohmann;
	
	json j_root = import_json_file(filename);
		
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
		
		cameras.push_back(cam);
	}
	return cameras;
}


void write_cameras_file(const std::string& filename, const camera_array& cameras) {
	// Does not use json.hpp, prints easier-to-read JSON.
	
	std::ofstream stream(filename);

	stream << "[";
	
	bool first = true;
	for(const camera& cam : cameras) {
		if(! first) stream << ",";
		else first = false;
		
		stream
			<< "\n{\n"
			<< "\t\"name\" : " << std::quoted(cam.name) << ",\n"
			<< "\t\"K\" :  [[" << frm(cam.intrinsic(0, 0)) << ", " << frm(cam.intrinsic(0, 1)) << ", " << frm(cam.intrinsic(0, 2)) << "],\n"
			<< "\t        ["   << frm(cam.intrinsic(1, 0)) << ", " << frm(cam.intrinsic(1, 1)) << ", " << frm(cam.intrinsic(1, 2)) << "],\n"
			<< "\t        ["   << frm(cam.intrinsic(2, 0)) << ", " << frm(cam.intrinsic(2, 1)) << ", " << frm(cam.intrinsic(2, 2)) << "]],\n"
			<< "\t\"Rt\" : [[" << frm(cam.extrinsic(0, 0)) << ", " << frm(cam.extrinsic(0, 1)) << ", " << frm(cam.extrinsic(0, 2)) << ", " << frm(cam.extrinsic(0, 3)) << "],\n"
			<< "\t        ["   << frm(cam.extrinsic(1, 0)) << ", " << frm(cam.extrinsic(1, 1)) << ", " << frm(cam.extrinsic(1, 2)) << ", " << frm(cam.extrinsic(1, 3)) << "],\n"
			<< "\t        ["   << frm(cam.extrinsic(2, 0)) << ", " << frm(cam.extrinsic(2, 1)) << ", " << frm(cam.extrinsic(2, 2)) << ", " << frm(cam.extrinsic(2, 3)) << "],\n"
			<< "\t        ["   << frm(cam.extrinsic(3, 0)) << ", " << frm(cam.extrinsic(3, 1)) << ", " << frm(cam.extrinsic(3, 2)) << ", " << frm(cam.extrinsic(3, 3)) << "]]\n"
			<< "}";
	}
	stream << "\n]";
}

/*
void write_cameras_file(const std::string& filename, const camera_array& cameras) {
	using namespace nlohmann;
	
	json j_root = json::array();
	
	for(const camera& cam : cameras) {
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
	
	export_json_file(j_root, filename);
}
*/


std::map<std::string, camera> cameras_map(const camera_array& arr) {
	std::map<std::string, camera> map;
	for(const camera& cam : arr) map[cam.name] = cam;
	return map;
}

void write_cameras_file(const std::string& filename, const std::map<std::string, camera>& map) {
	camera_array arr;
	for(const auto& kv : map) arr.push_back(kv.second);
	write_cameras_file(filename, arr);
}


}
