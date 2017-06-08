#include "lib/freenect2.h"
#include <iostream>
#include <stdexcept>

using namespace tlz;

int main() {
	using namespace libfreenect2;
	
	Freenect2 context;
	int count = context.enumerateDevices();
	std::cout << count << " Kinect devices found.\n";
	if(count == 0) throw std::runtime_error("Kinect not found");
		
	Freenect2Device* device = context.openDevice(0);
	if(device == nullptr) throw std::runtime_error("could not open device");
	
	device->start();
	device->close();
	delete device;
}
