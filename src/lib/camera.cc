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
	stream << std::setprecision(11) << std::setw(7) << std::setfill(' ') << f.value_;
	return stream;
}

}


mat44 camera::extrinsic() const {
	return mat44(
		rotation(0, 0), rotation(0, 1), rotation(0, 2), translation[0],
		rotation(1, 0), rotation(1, 1), rotation(1, 2), translation[1],
		rotation(2, 0), rotation(2, 1), rotation(2, 2), translation[2],
		0.0, 0.0, 0.0, 1.0
	);
}
	
camera_array read_cameras_file(const std::string& filename) {
	camera_array cameras;
	
	using namespace nlohmann;
	
	json j_root = import_json_file(filename);
		
	for(auto& j_cam : j_root) {
		camera cam;
		cam.name = j_cam["name"];
		cam.intrinsic = decode_mat(j_cam["K"]);
		
		mat44 extrinsic = decode_mat(j_cam["Rt"]);
		cam.rotation = extrinsic.get_minor<3, 3>(0, 0);
		
		auto t = extrinsic.get_minor<3, 1>(0, 3);
		cam.translation = vec3(t(0, 0), t(1, 0), t(2, 0));
				
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
			<< "\t\"Rt\" : [[" << frm(cam.rotation(0, 0)) << ", " << frm(cam.rotation(0, 1)) << ", " << frm(cam.rotation(0, 2)) << ", " << frm(cam.translation[0]) << "],\n"
			<< "\t        ["   << frm(cam.rotation(1, 0)) << ", " << frm(cam.rotation(1, 1)) << ", " << frm(cam.rotation(1, 2)) << ", " << frm(cam.translation[1]) << "],\n"
			<< "\t        ["   << frm(cam.rotation(2, 0)) << ", " << frm(cam.rotation(2, 1)) << ", " << frm(cam.rotation(2, 2)) << ", " << frm(cam.translation[2]) << "],\n"
			<< "\t        [" << frm(0.0) << ", " << frm(0.0) << ", " << frm(0.0) << ", " << frm(1.0) << "]]\n"
			<< "}";
	}
	stream << "\n]";
}


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
