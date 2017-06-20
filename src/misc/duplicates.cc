#include "../lib/args.h"
#include "../lib/dataset.h"
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <sstream>

using namespace tlz;

const bool use_diff = false;

// not exhaustive: only searching images with adjacent X

bool files_are_equal(const std::string& filename1, const std::string& filename2) {	
    std::ifstream file1(filename1, std::ifstream::ate | std::ifstream::binary);
    std::ifstream file2(filename2, std::ifstream::ate | std::ifstream::binary);
     
    if(file1.tellg() != file2.tellg()) return false;
    
	file1.seekg(0);
	file2.seekg(0);

    std::istreambuf_iterator<char> begin1(file1);
    std::istreambuf_iterator<char> begin2(file2);
	bool equal = std::equal(begin1, std::istreambuf_iterator<char>(), begin2);

	if(equal && use_diff) {
		std::ostringstream cmd;
		cmd << "diff " << std::quoted(filename1) << " " << std::quoted(filename2);
		int diff = std::system(cmd.str().c_str());
		if(diff != 0) throw std::runtime_error("diff disagrees");
	}

	return equal;
}

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json [dataset_group]");
	dataset datas = dataset_arg();
	std::string dataset_group_name = string_opt_arg("");
	
	dataset_group datag = datas.group(dataset_group_name);

	int duplicates_count = 0;
	for(int y = datas.y_min(); y <= datas.y_max(); y += datas.y_step()) {
		for(int x = datas.x_min() + datas.x_step(); x <= datas.x_max(); x += datas.x_step()) {
			view_index ind_prev(x - datas.x_step(), y), ind(x, y);
			
			dataset_view view = datag.view(ind);
			dataset_view view_prev = datag.view(ind_prev);
			
			if(files_are_equal(view.image_filename(), view_prev.image_filename())) {
				std::cout << "same image for " << ind_prev << " and " << ind << std::endl;
				++duplicates_count;
			}
			if(files_are_equal(view.depth_filename(), view_prev.depth_filename())) {
				std::cout << "same depth for " << ind_prev << " and " << ind << std::endl;
				++duplicates_count;
			}
		}
	}
	
	std::cout << "\nfound " << duplicates_count << " duplicates of out " << datas.x_count()*datas.y_count() << " images" << std::endl;
}
