#ifndef LICORNEA_KINECT_IR_TO_COLOR_SAMPLE_H_
#define LICORNEA_KINECT_IR_TO_COLOR_SAMPLE_H_

#include "../../lib/common.h"

namespace tlz {

template<typename Value>
struct ir_to_color_sample {
	Value value;
	vec2 color_coordinates;
	vec2 ir_coordinates;
	real color_depth;
	real ir_depth;
};

}

#endif
