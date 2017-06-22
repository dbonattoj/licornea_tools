#include <iostream>
#include <utility>
#include <vector>
#include <deque>
#include <string>
#include "lib/cg/references_grid.h"
#include "../lib/args.h"
#include "../lib/json.h"
#include "../lib/filesystem.h"
#include "../lib/dataset.h"
#include "../lib/image_io.h"

using namespace tlz;

const bool verbose = false;
const int max_reference_x_deviation = 5;


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json  horiz_key vert_key out_refgrid.json [dataset_group]");
	dataset datas = dataset_arg();
	int horizontal_key = int_arg();
	int vertical_key = int_arg();
	std::string out_refgrid_filename = out_filename_arg();
	std::string dataset_group_name = string_opt_arg("");
	
	dataset_group datag = datas.group(dataset_group_name);

	auto column_exists = [&](int x) {
		// check if image files for all views in this column exist
		for(int y = datag.set().y_min(); y <= datag.set().y_max(); y += datag.set().y_step()) {
			view_index idx(x, y);
			if(! file_exists(datag.view(idx).image_filename())) return false;
		}
		return true;
	};

	std::cout << "determining X coordinates of reference views" << std::endl;
	// reference views need to be in columns where all images exist
	// (cannot have missing files on vertical flow)
	std::deque<int> reference_x_positions;
	for(int x = datas.x_mid() - horizontal_key; x >= datas.x_min(); x -= horizontal_key) reference_x_positions.push_front(x);
	reference_x_positions.push_back(datas.x_mid());
	for(int x = datas.x_mid() + horizontal_key; x <= datas.x_max(); x += horizontal_key) reference_x_positions.push_back(x);
	auto reference_x_valid = [&](int x) {
		return datas.x_valid(x) && column_exists(x);
	};
	for(int& x : reference_x_positions) {		
		if(reference_x_valid(x)) continue;
		bool ok = false;
		for(int diff = 1; (diff < max_reference_x_deviation) && !ok; ++diff) {
			if(reference_x_valid(x + diff)) { x += diff; ok = true; }
			if(reference_x_valid(x - diff)) { x -= diff; ok = true; }
		}
		if(! ok)
			throw std::runtime_error("could not find valid X coordinate near " + std::to_string(x) + " for reference view");
	}
	
	
	std::cout << "determining Y coordinates of reference views" << std::endl;
	std::deque<int> reference_y_positions;
	for(int y = datas.y_mid() - vertical_key; y >= datas.y_min(); y -= vertical_key) reference_y_positions.push_front(y);
	reference_y_positions.push_back(datas.y_mid());
	for(int y = datas.y_mid() + vertical_key; y <= datas.y_max(); y += vertical_key) reference_y_positions.push_back(y);

	std::cout << "saving reference grid" << std::endl;
	references_grid grid;
	grid.x_indices.assign(reference_x_positions.begin(), reference_x_positions.end());
	grid.y_indices.assign(reference_y_positions.begin(), reference_y_positions.end());
	export_json_file(encode_references_grid(grid), out_refgrid_filename);
}

