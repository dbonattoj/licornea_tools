#ifndef LICORNEA_KINECT_REPROJECTION_H_
#define LICORNEA_KINECT_REPROJECTION_H_

#include "../../../lib/opencv.h"
#include "../../../lib/color.h"
#include "../kinect_intrinsics.h"
#include <vector>
#include <tuple>

namespace tlz {

class kinect_reprojection {
private:
	kinect_intrinsic_parameters intrinsics_;
	
public:
	explicit kinect_reprojection(const kinect_intrinsic_parameters&);
	
	vec2 distort_depth(vec2 undistorted) const;
	vec2 undistort_depth(vec2 distorted) const;
	vec2 reproject_depth_to_color(vec2 undistorted, real z) const;
	vec3 backproject_depth(vec2 undistorted, real z) const;
};

}

#endif
