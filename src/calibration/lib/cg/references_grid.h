#ifndef LICORNEA_CG_REFERENCES_GRID_H_
#define LICORNEA_CG_REFERENCES_GRID_H_

#include "../../../lib/json.h"
#include "../../../lib/args.h"
#include "../image_correspondence.h"
#include <string>

namespace tlz {

struct references_grid {
	std::vector<int> x_indices;
	std::vector<int> y_indices;
	
	std::size_t cols() const { return x_indices.size(); }
	std::size_t rows() const { return y_indices.size(); }
	
	std::size_t size() const { return cols() * rows(); }
	view_index view(std::ptrdiff_t col, std::ptrdiff_t row) const;
	bool has_view(const view_index&) const;
	
	bool is_valid() const { return x_indices.size() > 0; }
};

json encode_references_grid(const references_grid&);
references_grid decode_references_grid(const json&);

references_grid get_references_grid(const image_correspondences&);

references_grid references_grid_arg();


}

#endif
