#include "../lib/args.h"
#include "../lib/dataset.h"
#include "lib/image_correspondence.h"
#include "lib/feature_points.h"
#include <iostream>
#include <atomic>

using namespace tlz;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json cors.json");
	dataset datas = dataset_arg();
	image_correspondences cors = image_correspondences_arg();
	
	auto refs = get_reference_views(cors);
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
	
	auto all_views = datas.indices();
	const int view_feature_counts_hist_max = 10;
	std::vector<std::atomic<int>> view_feature_counts_hist(view_feature_counts_hist_max+2);
	#pragma omp parallel for
	for(std::ptrdiff_t i = 0; i < all_views.size(); ++i) {
		const view_index& idx = all_views[i];
		auto fpoints = feature_points_for_view(cors, idx);
		int count = fpoints.points.size();
		if(count > view_feature_counts_hist_max)
			view_feature_counts_hist[view_feature_counts_hist_max+1]++;
		else
			view_feature_counts_hist[count]++;
	}
	std::cout << "features per view:" << std::endl;
	for(int count = 0; count <= view_feature_counts_hist_max; ++count) {
		std::cout << count << " features: " << view_feature_counts_hist[count] << " views" << std::endl;
	}
	std::cout << "more features: " << view_feature_counts_hist[view_feature_counts_hist_max+1] << " views" << std::endl;
}
