#ifndef LICORNEA_OBJ_IMG_CORRESPONDENCE_H_
#define LICORNEA_OBJ_IMG_CORRESPONDENCE_H_

#include <json.hpp>
#include "json.h"
#include "opencv.h"
#include <array>
#include <vector>

namespace tlz {

template<std::size_t Obj_count, std::size_t Img_count>
struct obj_img_correspondence {
	std::array<vec3, Obj_count> object_coordinates;
	std::array<vec2, Img_count> image_coordinates;
};

template<std::size_t Obj_count, std::size_t Img_count>
using obj_img_correspondences = std::vector<obj_img_correspondence<Obj_count, Img_count>>;

template<std::size_t Obj_count, std::size_t Img_count>
using obj_img_correspondences_set = std::vector<obj_img_correspondences<Obj_count, Img_count>>;


template<std::size_t Obj_count, std::size_t Img_count>
obj_img_correspondence<Obj_count, Img_count> decode_obj_img_correspondence(const json&);

template<std::size_t Obj_count, std::size_t Img_count>
json encode_obj_img_correspondence(const obj_img_correspondence<Obj_count, Img_count>&);

template<std::size_t Obj_count, std::size_t Img_count>
json encode_obj_img_correspondences_set(const obj_img_correspondences_set<Obj_count, Img_count>&);

template<std::size_t Obj_count, std::size_t Img_count>
obj_img_correspondences_set<Obj_count, Img_count> decode_obj_img_correspondences_set(const json&);

struct obj_img_correspondences_set_dim {
	std::size_t obj_count = 0;
	std::size_t img_count = 0;
	obj_img_correspondences_set_dim() = default;
	obj_img_correspondences_set_dim(std::size_t obj_c, std::size_t img_c) :
		obj_count(obj_c), img_count(img_c) { }
};
obj_img_correspondences_set_dim decode_obj_img_correspondences_set_dim(const json&);

}

#include "obj_img_correspondence.tcc"

#endif
