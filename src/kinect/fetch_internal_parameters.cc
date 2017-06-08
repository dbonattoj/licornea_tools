#include "lib/args.h"
#include "lib/freenect2.h"
#include "lib/kinect_internal_parameters.h"
#include "lib/json.h"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdexcept>

using namespace tlz;

kinect_internal_parameters fetch_internal_parameters() {
	using namespace libfreenect2;
	
	Freenect2 context;
	int count = context.enumerateDevices();
	std::cout << count << " Kinect devices found.\n";
	if(count == 0) throw std::runtime_error("Kinect not found");
		
	Freenect2Device* device = context.openDevice(0);
	if(device == nullptr) throw std::runtime_error("could not open device");
	
	device->start();
	
	std::cout << "device serial number: " << device->getSerialNumber() << "\n";
	std::cout << "device firmware version: " << device->getFirmwareVersion() << "\n";
	Freenect2Device::ColorCameraParams color = device->getColorCameraParams();
	Freenect2Device::IrCameraParams ir = device->getIrCameraParams();
	device->close();
	delete device;
	
	return from_freenect2(color, ir);
}


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "out_internal_parameters.json");
	std::string out_internal_parameters_filename = out_filename_arg();
	
	std::cout << "fetching from Kinect" << std::endl;
	kinect_internal_parameters param = fetch_internal_parameters();
	
	std::cout << "saving to file" << std::endl;
	export_json_file(encode_kinect_internal_parameters(param), out_internal_parameters_filename);
}
