#include "camera.h"
#include <fstream>
#include <iterator>

std::vector<camera> read_cameras_file(const std::string& filename) {
	std::vector<camera> arr;
	std::ifstream input(filename);
	read_cameras(input, std::inserter(arr, arr.end()));
	return arr;
}


void write_cameras_file(const std::string& filename, const std::vector<camera>& arr) {
	std::ofstream output(filename);
	write_cameras(output, arr.begin(), arr.end());
}