#include "references_grid.h"
#include <algorithm>

namespace tlz {

view_index references_grid::view(std::ptrdiff_t col, std::ptrdiff_t row) const {
	return view_index(x_indices.at(col), y_indices.at(row));
}


bool references_grid::has_view(const view_index& idx) const {
	return std::any_of(x_indices.cbegin(), x_indices.cend(), [idx](const int& x){ return x == idx.x; })
		&& std::any_of(y_indices.cbegin(), y_indices.cend(), [idx](const int& y){ return y == idx.y; });
}


json encode_references_grid(const references_grid& grid) {
	json j_grid = json::object();
	j_grid["x_indices"] = grid.x_indices;
	j_grid["y_indices"] = grid.y_indices;
	return j_grid;
}


references_grid decode_references_grid(const json& j_grid) {
	references_grid grid;
	for(int x : j_grid["x_indices"]) grid.x_indices.push_back(x);
	for(int y : j_grid["y_indices"]) grid.y_indices.push_back(y);
	return grid;
}


references_grid get_references_grid(const image_correspondences& cors) {
	references_grid grid;
	
	auto ref_vws = get_reference_views_set(cors);
	
	std::vector<int> ref_x_positions, ref_y_positions;
	{
		std::set<int> ref_x_positions_set, ref_y_positions_set;
		for(const view_index& idx : ref_vws) {
			ref_x_positions_set.insert(idx.x);
			ref_y_positions_set.insert(idx.y);
		}
		for(int x : ref_x_positions) grid.x_indices.push_back(x);
		for(int y : ref_y_positions) grid.y_indices.push_back(y);
	}

	for(int row = 0; row < grid.rows(); ++row)
	for(int col = 0; col < grid.cols(); ++col) {
		view_index ref_idx = grid.view(col, row);
		if(ref_vws.find(ref_idx) == ref_vws.end())
			throw std::runtime_error("reference views are not arranged in a grid");
	}
	
	return grid;
}


references_grid references_grid_arg() {
	return decode_references_grid(json_arg());
}


}
