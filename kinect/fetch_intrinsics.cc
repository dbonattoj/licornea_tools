#include <json.hpp>
#include <libfreenect2/libfreenect2.hpp>
#include "lib/kinect_intrinsics.h"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdexcept>

using namespace libfreenect2;
using namespace tlz;

kinect_intrinsic_parameters fetch_intrinsic_parameters() {
	Freenect2 context;
	int count = context.enumerateDevices();
	std::cout << count << " Kinect devices found.\n";
	if(count == 0) throw std::runtime_error("Kinect not found");
		
	Freenect2Device* device = context.openDevice(0);
	if(device == nullptr) throw std::runtime_error("could not open device");
	
	device->start();
	
	kinect_intrinsic_parameters param;
	std::cout << "device serial number: " << device->getSerialNumber() << "\n";
	std::cout << "device firmware version: " << device->getFirmwareVersion() << "\n";
	param.color = device->getColorCameraParams();
	param.ir = device->getIrCameraParams();
	device->close();
	delete device;
	
	return param;
}


int main(int argc, const char* argv[]) {
	if(argc <= 1) {
		std::cout << "usage: " << argv[0] << " intrinsics.json" << std::endl;
		return EXIT_FAILURE;
	}
	const char* output_filename = argv[1];
	
	kinect_intrinsic_parameters param = fetch_intrinsic_parameters();
	
	std::ofstream str(output_filename);
	export_intrinsic_parameters(str, param);
}
