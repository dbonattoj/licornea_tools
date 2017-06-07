#include "feature_point.h"

namespace tlz {

feature_point decode_feature_point(const json& j_pt) {
	feature_point pt;
	pt.position[0] = j_pt["x"];
	pt.position[1] = j_pt["y"];
	pt.depth = get_or(j_pt, "depth", 0.0);
	pt.weight = get_or(j_pt, "weight", 1.0);
	return pt;
}


json encode_feature_point(const feature_point& pt) {
	json j_pt = json::object();
	j_pt["x"] = pt.position[0];
	j_pt["y"] = pt.position[1];
	if(pt.depth != 0.0) j_pt["depth"] = pt.depth;
	if(pt.weight != 1.0) j_pt["weight"] = pt.weight;
	return j_pt;
}

}
