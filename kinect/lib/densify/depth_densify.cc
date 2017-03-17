#include "depth_densify.h"
#include "depth_densify_mine.h"
#include "depth_densify_splat.h"

namespace tlz {

std::unique_ptr<depth_densify_base> make_depth_densify(const std::string& method) {
	if(method == "mine") return std::make_unique<depth_densify_mine>();
	else if(method == "splat") return std::make_unique<depth_densify_splat>();
	else throw std::invalid_argument("unknown depth densify method");
}


}
