#include "camera.h"
#include <istream>
#include <ostream>

bool read_camera(std::istream& input, camera& cam) {
	input >> cam.name;
	if(input.eof() || input.fail()) return false;
	
	if(cam.name == "0") { // extra last row (0 0 0 1) of previous extrinsic
		int unused;
		input >> unused >> unused >> unused;
		return read_camera(input, cam);
	}

	input >> cam.intrinsic(0, 0) >> cam.intrinsic(0, 1) >> cam.intrinsic(0, 2);
	input >> cam.intrinsic(1, 0) >> cam.intrinsic(1, 1) >> cam.intrinsic(1, 2);
	input >> cam.intrinsic(2, 0) >> cam.intrinsic(2, 1) >> cam.intrinsic(2, 2);
		
	Eigen_scalar unused[2]; // unused, always 0
	input >> unused[0] >> unused[1];

	input >> cam.rotation(0, 0) >> cam.rotation(0, 1) >> cam.rotation(0, 2) >> cam.translation(0);
	input >> cam.rotation(1, 0) >> cam.rotation(1, 1) >> cam.rotation(1, 2) >> cam.translation(1);
	input >> cam.rotation(2, 0) >> cam.rotation(2, 1) >> cam.rotation(2, 2) >> cam.translation(2);
	
	return true;
}

void write_camera(std::ostream& output, const camera& cam) {
	output << cam.name << "\n";
	output << cam.intrinsic(0, 0) << ' ' << cam.intrinsic(0, 1) << ' ' << cam.intrinsic(0, 2) << '\n';
	output << cam.intrinsic(1, 0) << ' ' << cam.intrinsic(1, 1) << ' ' << cam.intrinsic(1, 2) << '\n';
	output << cam.intrinsic(2, 0) << ' ' << cam.intrinsic(2, 1) << ' ' << cam.intrinsic(2, 2) << '\n';
	output << "0\n0\n";
	output << cam.rotation(0, 0) << ' ' << cam.rotation(0, 1) << ' ' << cam.rotation(0, 2) << ' ' << cam.translation(0) << '\n';
	output << cam.rotation(1, 0) << ' ' << cam.rotation(1, 1) << ' ' << cam.rotation(1, 2) << ' ' << cam.translation(1) << '\n';
	output << cam.rotation(2, 0) << ' ' << cam.rotation(2, 1) << ' ' << cam.rotation(2, 2) << ' ' << cam.translation(2) << '\n';
	output << '\n';
}
