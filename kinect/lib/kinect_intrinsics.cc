#include "kinect_intrinsics.h"
#include <ostream>
#include <json.hpp>

using namespace nlohmann;

namespace tlz {

void export_intrinsic_parameters(std::ostream& out, const kinect_intrinsic_parameters& param) {
	#define COLOR_PARAM(__field__) j["color"][#__field__] = param.color.__field__;
	#define IR_PARAM(__field__) j["ir"][#__field__] = param.ir.__field__;
	json j = {
		{"color", json::object()},
		{"ir", json::object()}
	};
	KINECT_INTRINSICS_COLOR_ELEMENTS;
	KINECT_INTRINSICS_IR_ELEMENTS;
	j >> out;
	#undef COLOR_PARAM
	#undef IR_PARAM
}



kinect_intrinsic_parameters import_intrinsic_parameters(std::istream& in) {
	#define COLOR_PARAM(__field__) param.color.__field__ = j["color"][#__field__];
	#define IR_PARAM(__field__) param.ir.__field__ = j["ir"][#__field__];
	json j;
	in >> j;
	kinect_intrinsic_parameters param;
	KINECT_INTRINSICS_COLOR_ELEMENTS;
	KINECT_INTRINSICS_IR_ELEMENTS;
	return param;
	#undef COLOR_PARAM
	#undef IR_PARAM
}

}
