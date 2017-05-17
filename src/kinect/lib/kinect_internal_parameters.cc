#include "kinect_internal_parameters.h"

namespace tlz {

json encode_kinect_internal_parameters(const kinect_internal_parameters& param) {
	#define COLOR_PARAM(__field__) j["color"][#__field__] = param.color.__field__;
	#define IR_PARAM(__field__) j["ir"][#__field__] = param.ir.__field__;
	json j = {
		{"color", json::object()},
		{"ir", json::object()}
	};
	KINECT_INTERNAL_PARAMETERS_COLOR_ELEMENTS;
	KINECT_INTERNAL_PARAMETERS_IR_ELEMENTS;
	#undef COLOR_PARAM
	#undef IR_PARAM
	return j;
}



kinect_internal_parameters decode_kinect_internal_parameters(const json& j) {
	#define COLOR_PARAM(__field__) param.color.__field__ = j["color"][#__field__];
	#define IR_PARAM(__field__) param.ir.__field__ = j["ir"][#__field__];
	kinect_internal_parameters param;
	KINECT_INTERNAL_PARAMETERS_COLOR_ELEMENTS;
	KINECT_INTERNAL_PARAMETERS_IR_ELEMENTS;
	return param;
	#undef COLOR_PARAM
	#undef IR_PARAM
}

}
