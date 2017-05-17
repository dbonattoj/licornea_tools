#ifndef LICORNEA_INTRINSICS_H_
#define LICORNEA_INTRINSICS_H_

#include "opencv.h"
#include "json.h"

namespace tlz {

struct intrinsics {
	mat33 K;
	mat33 K_inv;
	struct {
		real k1 = 0.0;
		real k2 = 0.0;
		real k3 = 0.0;
		real p1 = 0.0;
		real p2 = 0.0;
	
		std::vector<real> cv_coeffs() const {
			return { k1, k2, p1, p2, k3 };
		}
	} distortion;
	int width = 0;
	int height = 0;
};	

intrinsics decode_intrinsics(const json&);
json encode_intrinsics(const intrinsics&);

}

#endif

