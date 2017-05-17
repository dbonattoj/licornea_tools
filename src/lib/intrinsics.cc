#include "intrinsics.h"

namespace tlz {

intrinsics decode_intrinsics(const json& j_intr) {
	intrinsics intr;
	intr.K = decode_mat(j_intr["K"]);
	intr.K_inv = intr.K.inv();
	if(j_intr.count("distortion") == 1) {
		intr.distortion.k1 = j_intr["distortion"]["k1"];
		intr.distortion.k2 = j_intr["distortion"]["k2"];
		intr.distortion.p1 = j_intr["distortion"]["p1"];
		intr.distortion.p2 = j_intr["distortion"]["p1"];
		if(j_intr["distortion"].count("k3") == 1) intr.distortion.k3 = j_intr["distortion"]["k3"];
	}
	intr.width = j_intr["width"];
	intr.height = j_intr["height"];
	return intr;
}


json encode_intrinsics(const intrinsics& intr) {
	json j_intr;
	j_intr["K"] = encode_mat(intr.K);
	j_intr["distortion"] = json::object();
	j_intr["distortion"]["k1"] = intr.distortion.k1;
	j_intr["distortion"]["k2"] = intr.distortion.k2;
	j_intr["distortion"]["k3"] = intr.distortion.k3;
	j_intr["distortion"]["p1"] = intr.distortion.p1;
	j_intr["distortion"]["p2"] = intr.distortion.p2;
	j_intr["width"] = intr.width;
	j_intr["height"] = intr.height;
	return j_intr;
}


}
