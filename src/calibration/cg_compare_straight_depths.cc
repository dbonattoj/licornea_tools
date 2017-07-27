#include <iostream>
#include <utility>
#include <vector>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <set>
#include <map>
#include "lib/cg/straight_depths.h"
#include "../lib/args.h"
#include "../lib/misc.h"

using namespace tlz;

constexpr bool verbose = true;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "depths1.json depths2.json [depths.txt]");
	straight_depths depths1 = straight_depths_arg();
	straight_depths depths2 = straight_depths_arg();
	std::string out_depths_filename = out_filename_opt_arg("");
	
	// find common features
	std::set<std::string> common_features;
	for(const auto& kv : depths1) {
		const std::string& feature_name = kv.first;
		if(depths2.find(feature_name) != depths2.end()) common_features.insert(feature_name);
	}
	std::cout << "depths1 count: " << depths1.size() << std::endl;
	std::cout << "depths2 count: " << depths2.size() << std::endl;
	std::cout << "common count:  " << common_features.size() << std::endl;
	
	// calculate rms error
	real rms_error = 0.0;
	for(const std::string& feature_name : common_features) {
		real d1 = depths1.at(feature_name);
		real d2 = depths2.at(feature_name);
		rms_error += sq(d1 - d2);
	}
	rms_error = std::sqrt(rms_error / common_features.size());
	std::cout << "rms_error: " << rms_error << std::endl;
	
	// print to file
	if(! out_depths_filename.empty()) {
		std::ofstream stream(out_depths_filename);
		stream << std::setprecision(10);
		for(const std::string& feature_name : common_features) {
			real d1 = depths1.at(feature_name);
			real d2 = depths2.at(feature_name);
			stream << feature_name << ' ' << d1 << ' ' << d2 << '\n';
		}
	}
}
