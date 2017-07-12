#include "../lib/args.h"
#include "../lib/dataset.h"
#include "../lib/filesystem.h"
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <string>

using namespace tlz;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json [dataset_group]");
	dataset datas = dataset_arg();
	std::string dataset_group_name = string_opt_arg("");
	
	dataset_group datag = datas.group(dataset_group_name);

	for(view_index idx : datas.indices()) {
		std::cout << '.' << std::flush;
		dataset_view view = datag.view(idx);
		bool have_image = file_exists(view.image_filename());
		bool have_depth = file_exists(view.depth_filename());
		
		if(! have_image) std::cout << "\nmissing image " << idx << " (" << view.image_filename() << ")\n";
		if(! have_depth) std::cout << "\nmissing depth " << idx << " (" << view.depth_filename() << ")\n";
	}
	std::cout << std::endl;
}
