#ifndef LICORNEA_FREENECT2_H_
#define LICORNEA_FREENECT2_H_

#include <libfreenect2/libfreenect2.hpp>
#include "kinect_intrinsics.h"
#include <utility>

namespace tlz {

using freenect2_color_params = libfreenect2::Freenect2Device::ColorCameraParams;
using freenect2_ir_params = libfreenect2::Freenect2Device::IrCameraParams;


std::pair<freenect2_color_params, freenect2_ir_params> to_freenect2(const kinect_intrinsic_parameters&);
kinect_intrinsic_parameters from_freenect2(const freenect2_color_params&, const freenect2_ir_params&);


}

#endif
