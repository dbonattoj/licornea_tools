#include "obj_img_correspondence.h"

namespace tlz {

obj_img_correspondences_set_dim decode_obj_img_correspondences_set_dim(const json& j_set) {
	const json& j_cor = j_set.front().front();
	obj_img_correspondences_set_dim dim { 0, 0 };
	for(auto it = j_cor.begin(); it != j_cor.end(); ++it) {
		const std::string& key = it.key();
		if(key.substr(0, 6) == "object") dim.obj_count++;
		else if(key.substr(0, 5) == "image") dim.img_count++;
	}
	return dim;
}

}
