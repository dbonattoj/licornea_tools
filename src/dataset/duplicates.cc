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

bool files_are_equal(const std::string& filename1, const std::string& filename2) {
	if(file_size(filename1) != file_size(filename2)) return false;
 
    std::ifstream file1(filename1, std::ifstream::binary);
    std::ifstream file2(filename2, std::ifstream::binary);
     
    std::istreambuf_iterator<char> begin1(file1);
    std::istreambuf_iterator<char> begin2(file2);
	bool equal = std::equal(begin1, std::istreambuf_iterator<char>(), begin2);

	return equal;
}

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json +x/-x/+y/-y [bad_files.txt] [dataset_group]");
	dataset datas = dataset_arg();
	std::string mode = enum_arg({ "+x", "-x", "+y", "-y" });
	std::string bad_files_filename = out_filename_opt_arg("");
	std::string dataset_group_name = string_opt_arg("");
	
	dataset_group datag = datas.group(dataset_group_name);

	int duplicates_count = 0;
	std::vector<std::string> bad_files;
	
	auto cmp = [&](const view_index& prev, const view_index& cur) {
		dataset_view view_prev = datag.view(prev);
		dataset_view view_cur = datag.view(cur);
				
		std::string prev_image_filename = view_prev.image_filename();		
		std::string cur_image_filename = view_cur.image_filename(); 
		if(file_exists(prev_image_filename) && file_exists(cur_image_filename) && files_are_equal(prev_image_filename, cur_image_filename)) {
			duplicates_count++;
			bad_files.push_back(cur_image_filename);
			std::cout << "same images: " << prev << " and " << cur << std::endl;
		}
			
		std::string prev_depth_filename = view_prev.depth_filename();
		std::string cur_depth_filename = view_cur.depth_filename(); 
		if(file_exists(prev_depth_filename) && file_exists(cur_depth_filename) && files_are_equal(prev_depth_filename, cur_depth_filename)) {
			duplicates_count++;
			bad_files.push_back(cur_depth_filename);
			std::cout << "same depths: " << prev << " and " << cur << std::endl;
		}
	};

	if(mode == "+x") {
		for(int y = datas.y_min(); y <= datas.y_max(); y += datas.y_step()) {
			for(int x = datas.x_min() + datas.x_step(); x <= datas.x_max(); x += datas.x_step()) {
				view_index ind_prev(x - datas.x_step(), y), ind(x, y);
				cmp(ind_prev, ind);
			}
		}
			
	} else if(mode == "-x") {
		for(int y = datas.y_min(); y <= datas.y_max(); y += datas.y_step()) {
			for(int x = datas.x_max() - datas.x_step(); x >= datas.x_min(); x -= datas.x_step()) {
				view_index ind_prev(x + datas.x_step(), y), ind(x, y);
				cmp(ind_prev, ind);
			}
		}	
		
	} else if(mode == "+y") {
		// TODO
		
	} else if(mode == "-y") {
		// TODO
		
	}
	
	std::cout << "\nfound " << duplicates_count << " duplicates of out " << 2*datas.x_count()*datas.y_count() << " images" << std::endl;

	if(! bad_files_filename.empty()) {
		std::ofstream stream(bad_files_filename);
		for(const std::string& filename : bad_files) stream << filename << "\n";
		stream.close();
	}
}
