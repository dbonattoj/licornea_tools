#ifndef LICORNEA_IMAGE_CORRESPONDENCE_H_
#define LICORNEA_IMAGE_CORRESPONDENCE_H_

#include "json.h"
#include "eigen.h"
#include "dataset.h"
#include <map>
#include <vector>
#include <utility>

namespace tlz {

// TODO add "reference". needs point for that view needs to exists foreach feature

struct image_correspondence_feature {	
	using view_index_type = view_index;
	
	real depth = 0.0;
	std::map<view_index_type, Eigen_vec2> points;
	// when iterating (--> encoding into json array), gets sorted by index
};

image_correspondence_feature decode_image_correspondence_feature(const json&);
json encode_image_correspondence_feature(const image_correspondence_feature&); 


}

#endif
