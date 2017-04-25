#include "camera.h"
#include <fstream>
#include <iterator>

namespace tlz {

std::vector<camera> read_cameras_file(const std::string& filename) {
	std::vector<camera> arr;
	read_cameras(filename, std::inserter(arr, arr.end()));
	return arr;
}


std::map<std::string, camera> cameras_map(const std::vector<camera>& arr) {
	std::map<std::string, camera> map;
	for(const camera& cam : arr) map[cam.name] = cam;
	return map;
}


void write_cameras_file(const std::string& filename, const std::vector<camera>& arr) {
	write_cameras(filename, arr.begin(), arr.end());
}


void write_cameras_file(const std::string& filename, const std::map<std::string, camera>& map) {
	std::vector<camera> arr;
	for(const auto& kv : map) arr.push_back(kv.second);
	write_cameras_file(filename, arr);
}


}
