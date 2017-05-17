#ifndef LICORNEA_WITH_LIBFREENECT2
#error freenect2.h can only be included when building with Freenect2
#endif

#ifndef LICORNEA_FREENECT2_H_
#define LICORNEA_FREENECT2_H_

#include <libfreenect2/libfreenect2.hpp>
#include "kinect_internal_parameters.h"
#include <utility>

namespace tlz {

using freenect2_color_params = libfreenect2::Freenect2Device::ColorCameraParams;
using freenect2_ir_params = libfreenect2::Freenect2Device::IrCameraParams;


std::pair<freenect2_color_params, freenect2_ir_params> to_freenect2(const kinect_internal_parameters&);
kinect_internal_parameters from_freenect2(const freenect2_color_params&, const freenect2_ir_params&);


}

#endif
