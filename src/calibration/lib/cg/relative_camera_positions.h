#ifndef LICORNEA_RELATIVE_CAMERA_POSITIONS_H_
#define LICORNEA_RELATIVE_CAMERA_POSITIONS_H_

#include "../../../lib/common.h"
#include "../../../lib/json.h"
#include <utility>
#include <map>
#include <vector>

namespace tlz {
	
struct relative_camera_positions {
	using reference_view_index = view_index;
	using target_view_index = view_index;
	using key_type = std::pair<reference_view_index, target_view_index>;

	using reference_target_positions_type = std::map<reference_view_index, std::vector<std::pair<target_view_index, vec2>>>;
	using target_reference_positions_type = std::map<target_view_index, std::vector<std::pair<reference_view_index, vec2>>>;

	std::map<key_type, vec2> positions;
	
	reference_target_positions_type to_reference_target_positions() const;
	target_reference_positions_type to_target_reference_positions() const;
	
	vec2& position(const reference_view_index& ref_idx, const target_view_index& target_idx)
		{ return positions[std::make_pair(ref_idx, target_idx)]; }
	const vec2& position(const reference_view_index& ref_idx, const target_view_index& target_idx) const
		{ return positions.at(std::make_pair(ref_idx, target_idx)); }
};

json encode_relative_camera_positions(const relative_camera_positions&);
relative_camera_positions decode_relative_camera_positions(const json&);

std::vector<view_index> get_reference_views(const relative_camera_positions&);
std::vector<view_index> get_target_views(const relative_camera_positions&);



relative_camera_positions relative_camera_positions_arg();

}

#endif
