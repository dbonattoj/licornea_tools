#include "kinect_intrinsics.h"
#include <ostream>
#include <json.hpp>

using namespace nlohmann;

#define PARAMS \
	COLOR_PARAM(fx); \
	COLOR_PARAM(fy); \
	COLOR_PARAM(cx); \
	COLOR_PARAM(cy); \
	COLOR_PARAM(shift_d); \
	COLOR_PARAM(shift_m); \
	COLOR_PARAM(mx_x3y0); \
	COLOR_PARAM(mx_x0y3); \
	COLOR_PARAM(mx_x2y1); \
	COLOR_PARAM(mx_x1y2); \
	COLOR_PARAM(mx_x2y0); \
	COLOR_PARAM(mx_x0y2); \
	COLOR_PARAM(mx_x1y1); \
	COLOR_PARAM(mx_x1y0); \
	COLOR_PARAM(mx_x0y1); \
	COLOR_PARAM(mx_x0y0); \
	COLOR_PARAM(my_x3y0); \
	COLOR_PARAM(my_x0y3); \
	COLOR_PARAM(my_x2y1); \
	COLOR_PARAM(my_x1y2); \
	COLOR_PARAM(my_x2y0); \
	COLOR_PARAM(my_x0y2); \
	COLOR_PARAM(my_x1y1); \
	COLOR_PARAM(my_x1y0); \
	COLOR_PARAM(my_x0y1); \
	COLOR_PARAM(my_x0y0); \
	IR_PARAM(fx); \
	IR_PARAM(fy); \
	IR_PARAM(cx); \
	IR_PARAM(cy); \
	IR_PARAM(k1); \
	IR_PARAM(k2); \
	IR_PARAM(k3); \
	IR_PARAM(p1); \
	IR_PARAM(p2);

namespace tlz {

void export_intrinsic_parameters(std::ostream& out, const kinect_intrinsic_parameters& param) {
	#define COLOR_PARAM(__field__) j["color"][#__field__] = param.color.__field__;
	#define IR_PARAM(__field__) j["ir"][#__field__] = param.ir.__field__;
	json j = {
		{"color", json::object()},
		{"ir", json::object()}
	};
	PARAMS;
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
	PARAMS;
	return param;
	#undef COLOR_PARAM
	#undef IR_PARAM
}

}
