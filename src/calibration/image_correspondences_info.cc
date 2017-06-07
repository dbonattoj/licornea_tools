#include "../lib/args.h"
#include "lib/image_correspondence.h"
#include <iostream>

using namespace tlz;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "cors.json");
	image_correspondences cors = image_correspondences_arg();
	
	auto refs = reference_views(cors);
	std::cout << refs.size() << " reference views:\n";
	for(const view_index& idx : refs) std::cout << "    " << idx << "\n";
	std::cout << cors.features.size() << " features total\n";
	std::cout << "dataset group: " << cors.dataset_group << "\n";
	for(const view_index& idx : refs) {
		std::cout << "\nreference " << idx << ":\n";
		image_correspondences ref_cors = image_correspondences_with_reference(cors, idx);
		std::cout << "    " << ref_cors.features.size() << " features\n";
		for(const auto& kv : ref_cors.features)
			std::cout << "    " << kv.first << ": " << kv.second.points.size() << " views\n";
	}
}
