#include "camera.h"
#include <fstream>
#include <iterator>

namespace tlz {

std::vector<camera> read_cameras_file(const std::string& filename) {
	std::vector<camera> arr;
	std::ifstream input(filename);
	read_cameras(input, std::inserter(arr, arr.end()));
	return arr;
}


std::map<std::string, camera> cameras_map(const std::vector<camera>& arr) {
	std::map<std::string, camera> map;
	for(const camera& cam : arr) map[cam.name] = cam;
	return map;
}


void write_cameras_file(const std::string& filename, const std::vector<camera>& arr) {
	std::ofstream output(filename);
	write_cameras(output, arr.begin(), arr.end());
}


void write_cameras_file(const std::string& filename, const std::map<std::string, camera>& map) {
	std::vector<camera> arr;
	for(auto pair : map) arr.push_back(pair.second);
	write_cameras_file(filename, arr);
}


}
