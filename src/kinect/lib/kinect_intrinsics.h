#ifndef LICORNEA_KINECT_INTRINSICS_H_
#define LICORNEA_KINECT_INTRINSICS_H_

#include <iosfwd>
#include <string>
#include "../../lib/common.h"

#define KINECT_INTRINSICS_COLOR_ELEMENTS \
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
	COLOR_PARAM(my_x0y0);
	
#define KINECT_INTRINSICS_IR_ELEMENTS \
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

#define COLOR_PARAM(__field__) real __field__ = 0.0;
#define IR_PARAM(__field__) real __field__ = 0.0;

struct kinect_intrinsic_parameters {
	struct {
		KINECT_INTRINSICS_COLOR_ELEMENTS
	} color;
	struct {
		KINECT_INTRINSICS_IR_ELEMENTS
	} ir;
};

#undef COLOR_PARAM
#undef IR_PARAM

void export_intrinsic_parameters(std::ostream&, const kinect_intrinsic_parameters&);
kinect_intrinsic_parameters import_intrinsic_parameters(std::istream&);

}

#endif
