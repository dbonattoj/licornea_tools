#ifdef LICORNEA_WITH_LIBFREENECT2

#include "freenect2.h"

namespace tlz {
	
std::pair<freenect2_color_params, freenect2_ir_params> to_freenect2(const kinect_internal_parameters& par) {
	freenect2_color_params color;
	freenect2_ir_params ir;
	
	#define COLOR_PARAM(__field__) color.__field__ = par.color.__field__;
	#define IR_PARAM(__field__) ir.__field__ = par.ir.__field__;
	KINECT_INTERNAL_PARAMETERS_COLOR_ELEMENTS;
	KINECT_INTERNAL_PARAMETERS_IR_ELEMENTS;
	#undef COLOR_PARAM
	#undef IR_PARAM
	
	return std::make_pair(color, ir);
}


kinect_internal_parameters from_freenect2(const freenect2_color_params& color, const freenect2_ir_params& ir) {
	kinect_internal_parameters par;
	
	#define COLOR_PARAM(__field__) par.color.__field__ = color.__field__;
	#define IR_PARAM(__field__) par.ir.__field__ = ir.__field__;
	KINECT_INTERNAL_PARAMETERS_COLOR_ELEMENTS;
	KINECT_INTERNAL_PARAMETERS_IR_ELEMENTS;
	#undef COLOR_PARAM
	#undef IR_PARAM

	return par;
}

}

#endif
