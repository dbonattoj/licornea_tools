#include "straight_depths.h"

namespace tlz {

json encode_straight_depths(const straight_depths& depths) {
	json j_depths = json::object();
	for(const auto& kv : depths) {
		const std::string& feature_name = kv.first;
		const straight_depth& depth = kv.second;
		json j_depth = json::object();
		if(std::isnan(depth.depth)) continue;
		j_depth["depth"] = depth.depth;
		j_depth["confidence"] = depth.confidence;
		j_depths[feature_name] = j_depth;
	}
	return j_depths;
}


straight_depths decode_straight_depths(const json& j_depths) {
	straight_depths depths;
	for(auto it = j_depths.begin(); it != j_depths.end(); ++it) {
		const std::string& feature_name = it.key();
		const json& j_depth = it.value();
		if(j_depth.is_number())
			depths[feature_name] = straight_depth(real(j_depth));
		else
			depths[feature_name] = straight_depth(j_depth["depth"], get_or(j_depth, "confidence", 1.0));
	}
	return depths;
}


straight_depths straight_depths_arg() {
	return decode_straight_depths(json_arg());
}


}
