#ifndef LICORNEA_IMAGE_CORRESPONDENCE_H_
#define LICORNEA_IMAGE_CORRESPONDENCE_H_

#include "../../lib/json.h"
#include "../../lib/dataset.h"
#include "../../lib/args.h"
#include <map>
#include <vector>
#include <utility>

namespace tlz {

struct image_correspondence_feature {
	std::map<view_index, vec2> points;
	std::map<view_index, real> point_depths;
	std::map<view_index, real> point_weights;
	// when iterating (--> encoding into json array), gets sorted by index
};

struct image_correspondences {
	view_index reference;
	std::string dataset_group;
	std::map<std::string, image_correspondence_feature> features;
};

image_correspondence_feature decode_image_correspondence_feature(const json&);
json encode_image_correspondence_feature(const image_correspondence_feature&); 

image_correspondences decode_image_correspondences(const json&);
json encode_image_correspondences(const image_correspondences&);

inline image_correspondences image_correspondences_arg()
	{ return decode_image_correspondences(json_arg()); }

}

#endif
