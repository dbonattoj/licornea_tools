#ifndef LICORNEA_OBJ_IMG_CORRESPONDENCE_H_
#define LICORNEA_OBJ_IMG_CORRESPONDENCE_H_

#include <json.hpp>
#include "../../lib/json.h"
#include "../../lib/opencv.h"
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

}

#include "obj_img_correspondence.tcc"

#endif
