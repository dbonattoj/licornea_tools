#include "relative_camera_positions.h"
#include "../../../lib/string.h"
#include <set>

namespace tlz {


namespace {
	
std::string encode_key_(const relative_camera_positions::key_type& key) {
	return encode_view_index(key.first) + ";" + encode_view_index(key.second);
}

relative_camera_positions::key_type decode_key_(const std::string& encoded_key) {
	auto arr = explode(';', encoded_key);
	view_index ref_idx = decode_view_index(arr[0]);
	view_index target_idx = decode_view_index(arr[1]);
	return { ref_idx, target_idx };
}	
	
}

auto relative_camera_positions::to_reference_target_positions() const -> reference_target_positions_type {
	reference_target_positions_type map;
	for(const auto& kv : positions) {
		const view_index& ref_idx = kv.first.first;
		const view_index& target_idx = kv.first.second;
		const vec2& pos = kv.second;
		map[ref_idx].emplace_back(target_idx, pos);
	}
	return map;
}


auto relative_camera_positions::to_target_reference_positions() const -> target_reference_positions_type {
	target_reference_positions_type map;
	for(const auto& kv : positions) {
		const view_index& ref_idx = kv.first.first;
		const view_index& target_idx = kv.first.second;
		const vec2& pos = kv.second;
		map[target_idx].emplace_back(ref_idx, pos);
	}
	return map;
	
}


std::vector<view_index> get_reference_views(const relative_camera_positions& rcpos) {
	std::set<view_index> set;
	for(const auto& kv : rcpos.positions) set.insert(kv.first.first);
	return std::vector<view_index>(set.begin(), set.end());
}


std::vector<view_index> get_target_views(const relative_camera_positions& rcpos) {
	std::set<view_index> set;
	for(const auto& kv : rcpos.positions) set.insert(kv.first.second);
	return std::vector<view_index>(set.begin(), set.end());	
}


json encode_relative_camera_positions(const relative_camera_positions& rcpos) {
	json j_rcpos = json::object();
	for(const auto& kv : rcpos.positions) {
		const auto& key = kv.first;
		const vec2& pos = kv.second;
		j_rcpos[encode_key_(key)] = encode_mat(pos);
	}
	return j_rcpos;
}


relative_camera_positions decode_relative_camera_positions(const json& j_rcpos) {
	relative_camera_positions rcpos;
	for(auto it = j_rcpos.begin(); it != j_rcpos.end(); ++it) {
		auto key = decode_key_(it.key());
		vec2 pos = decode_mat(it.value());
		rcpos.positions[key] = pos;
	}
	return rcpos;
}

relative_camera_positions relative_camera_positions_arg() {
	return decode_relative_camera_positions(json_arg());
}

}
