#include "../lib/common.h"
#include "../lib/obj_img_correspondence.h"
#include "../lib/json.h"
#include "../lib/args.h"
#include <string>

using namespace tlz;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "in1.json in2.json out.json [1/2]");
	std::string cors1_filename = in_filename_arg();
	std::string cors2_filename = in_filename_arg();
	std::string out_cors_filename = out_filename_arg();
	int which = int_opt_arg(1) - 1;

	json cors1 = import_json_file(cors1_filename);
	json cors2 = import_json_file(cors2_filename);

	obj_img_correspondences_set_dim dim1 = decode_obj_img_correspondences_set_dim(cors1);
	obj_img_correspondences_set_dim dim2 = decode_obj_img_correspondences_set_dim(cors2);
	
	if(dim1.obj_count != 1 || dim2.obj_count != 1) throw std::runtime_error("unsupported dims (obj_count != 1)");

	if(dim1.img_count == 1 && dim2.img_count == 1) {
		obj_img_correspondences_set<1, 1> set1 = decode_obj_img_correspondences_set<1, 1>(cors1);
		obj_img_correspondences_set<1, 1> set2 = decode_obj_img_correspondences_set<1, 1>(cors2);
		obj_img_correspondences_set<1, 1> out_set = set1;
		out_set.insert(out_set.end(), set2.cbegin(), set2.cend());
		export_json_file(encode_obj_img_correspondences_set(out_set), out_cors_filename);
	} else if(dim1.img_count == 2 && dim2.img_count == 2) {
		obj_img_correspondences_set<1, 2> set1 = decode_obj_img_correspondences_set<1, 2>(cors1);
		obj_img_correspondences_set<1, 2> set2 = decode_obj_img_correspondences_set<1, 2>(cors2);
		obj_img_correspondences_set<1, 2> out_set = set1;
		out_set.insert(out_set.end(), set2.cbegin(), set2.cend());
		export_json_file(encode_obj_img_correspondences_set(out_set), out_cors_filename);
	} else if(dim1.img_count + dim2.img_count == 3) {
		obj_img_correspondences_set<1, 1> out_set;
		
		auto insert_from_double = [&out_set](const obj_img_correspondences_set<1, 2>& set, int which) {
			for(const auto& cors : set) {
				out_set.emplace_back();
				auto& out_cors = out_set.back();
				for(const auto& cor : cors) {
					out_cors.emplace_back();
					auto& out_cor = out_cors.back();
					out_cor.object_coordinates[0] = cor.object_coordinates[0];
					out_cor.image_coordinates[0] = cor.image_coordinates[which];
				}
			}
		};
		
		if(dim1.img_count == 2) {
			obj_img_correspondences_set<1, 2> set1 = decode_obj_img_correspondences_set<1, 2>(cors1);
			obj_img_correspondences_set<1, 1> set2 = decode_obj_img_correspondences_set<1, 1>(cors2);

			insert_from_double(set1, which);
			out_set.insert(out_set.end(), set2.cbegin(), set2.cend());
			
		} else if(dim2.img_count == 2) {
			obj_img_correspondences_set<1, 1> set1 = decode_obj_img_correspondences_set<1, 1>(cors1);
			obj_img_correspondences_set<1, 2> set2 = decode_obj_img_correspondences_set<1, 2>(cors2);
			
			out_set.insert(out_set.end(), set1.cbegin(), set1.cend());
			insert_from_double(set2, which);
		} else {
			throw std::runtime_error("unsupported dims");
		}
		
		export_json_file(encode_obj_img_correspondences_set(out_set), out_cors_filename);
	} else {
		throw std::runtime_error("unsupported dims");
	}
}
