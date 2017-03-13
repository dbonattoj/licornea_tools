#ifndef LICORNEA_KINECT_INTRINSICS_H_
#define LICORNEA_KINECT_INTRINSICS_H_

#include <libfreenect2/libfreenect2.hpp>
#include <iosfwd>
#include <string>

namespace tlz {

struct kinect_intrinsic_parameters {
	libfreenect2::Freenect2Device::ColorCameraParams color;
	libfreenect2::Freenect2Device::IrCameraParams ir;
};

void export_intrinsic_parameters(std::ostream&, const kinect_intrinsic_parameters&);
kinect_intrinsic_parameters import_intrinsic_parameters(std::istream&);

}

#endif
