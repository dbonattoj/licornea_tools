#ifndef LICORNEA_KINECT_REMAPPING_H_
#define LICORNEA_KINECT_REMAPPING_H_

#include "../../lib/opencv.h"
#include "../../lib/color.h"
#include "kinect_internal_parameters.h"

namespace tlz {

class kinect_remapping {
protected:
	kinect_internal_parameters internal_parameters_;

public:
	explicit kinect_remapping(const kinect_internal_parameters&);

	vec2 distort_ir(vec2 undistorted) const;
	vec2 undistort_ir(vec2 distorted) const;
	
	vec2 map_ir_to_color(vec2 undistorted_ir, real ir_z) const;
};

}

#endif
