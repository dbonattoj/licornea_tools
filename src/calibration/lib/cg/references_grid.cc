#include "references_grid.h"

namespace tlz {

view_index references_grid::view(std::ptrdiff_t col, std::ptrdiff_t row) const {
	return view_index(x_indices.at(col), y_indices.at(row));
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

references_grid references_grid_arg() {
	return decode_references_grid(json_arg());
}


}
