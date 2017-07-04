#ifndef LICORNEA_KINECT_REPROJECTION_PARAMETERS_H_
#define LICORNEA_KINECT_REPROJECTION_PARAMETERS_H_

#include "../../lib/opencv.h"
#include "../../lib/json.h"
#include "../../lib/intrinsics.h"

namespace tlz {

struct kinect_reprojection_parameters {	
	struct depth_offset_polyfit {
		real x0y0 = 0.0;
		
		real x1y0 = 0.0;
		real x0y1 = 0.0;
		
		real x2y0 = 0.0;
		real x0y2 = 0.0;
		real x1y1 = 0.0;
		
		real operator()(vec2 xy) const;
		
		bool is_none() const;
		explicit operator bool () const { return ! is_none(); }
	};

	intrinsics ir_intrinsics;
	intrinsics color_intrinsics;
	mat33 rotation;
	vec3 translation;
	depth_offset_polyfit ir_depth_offset;
	depth_offset_polyfit color_depth_offset;
};

json encode_kinect_reprojection_parameters(const kinect_reprojection_parameters&);
kinect_reprojection_parameters decode_kinect_reprojection_parameters(const json&);

}

#endif
