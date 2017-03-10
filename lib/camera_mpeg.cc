#include "camera_mpeg.h"
#include "camera.h"
#include <iomanip>
#include <istream>
#include <ostream>

bool read_camera_mpeg(std::istream& input, camera& cam, bool convert) {
	input >> cam.name;
	if(input.eof() || input.fail()) return false;
	
	if(cam.name == "0") { // extra last row (0 0 0 1) of previous extrinsic
		int unused;
		input >> unused >> unused >> unused;
		return read_camera_mpeg(input, cam, convert);
	}

	input >> cam.intrinsic(0, 0) >> cam.intrinsic(0, 1) >> cam.intrinsic(0, 2);
	input >> cam.intrinsic(1, 0) >> cam.intrinsic(1, 1) >> cam.intrinsic(1, 2);
	input >> cam.intrinsic(2, 0) >> cam.intrinsic(2, 1) >> cam.intrinsic(2, 2);
		
	Eigen_scalar unused[2]; // unused, always 0
	input >> unused[0] >> unused[1];

	input >> cam.extrinsic(0, 0) >> cam.extrinsic(0, 1) >> cam.extrinsic(0, 2) >> cam.extrinsic(0, 3);
	input >> cam.extrinsic(1, 0) >> cam.extrinsic(1, 1) >> cam.extrinsic(1, 2) >> cam.extrinsic(1, 3);
	input >> cam.extrinsic(2, 0) >> cam.extrinsic(2, 1) >> cam.extrinsic(2, 2) >> cam.extrinsic(2, 3);
	cam.extrinsic(3, 0) = 0.0; cam.extrinsic(3, 1) = 0.0; cam.extrinsic(3, 2) = 0.0; cam.extrinsic(3, 3) = 1.0;
	
	if(convert) cam.translation() = -(cam.rotation() * cam.translation());
		
	return true;
}

void write_camera_mpeg(std::ostream& output, const camera& orig_cam, bool convert) {
	camera cam = orig_cam;
	if(convert) cam.translation() = -(cam.rotation().inverse() * cam.translation());

	output << std::setprecision(16);
	output << cam.name << "\n";
	output << cam.intrinsic(0, 0) << ' ' << cam.intrinsic(0, 1) << ' ' << cam.intrinsic(0, 2) << '\n';
	output << cam.intrinsic(1, 0) << ' ' << cam.intrinsic(1, 1) << ' ' << cam.intrinsic(1, 2) << '\n';
	output << cam.intrinsic(2, 0) << ' ' << cam.intrinsic(2, 1) << ' ' << cam.intrinsic(2, 2) << '\n';
	output << "0\n0\n";
	output << cam.extrinsic(0, 0) << ' ' << cam.extrinsic(0, 1) << ' ' << cam.extrinsic(0, 2) << ' ' << cam.extrinsic(0, 3) << '\n';
	output << cam.extrinsic(1, 0) << ' ' << cam.extrinsic(1, 1) << ' ' << cam.extrinsic(1, 2) << ' ' << cam.extrinsic(1, 3) << '\n';
	output << cam.extrinsic(2, 0) << ' ' << cam.extrinsic(2, 1) << ' ' << cam.extrinsic(2, 2) << ' ' << cam.extrinsic(2, 3) << '\n';
	output << '\n';
}
