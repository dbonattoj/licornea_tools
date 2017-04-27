#ifndef LICORNEA_IMAGE_CORRESPONDENCE_H_
#define LICORNEA_IMAGE_CORRESPONDENCE_H_

#include "../../lib/json.h"
#include "../../lib/eigen.h"
#include "../../lib/dataset.h"
#include <map>
#include <vector>
#include <utility>

namespace tlz {

// TODO add "reference". needs point for that view needs to exists foreach feature

struct image_correspondence_feature {		
	real depth = 0.0;
	std::map<view_index, Eigen_vec2> points;
	// when iterating (--> encoding into json array), gets sorted by index
};

struct image_correspondences {
	view_index reference;
	std::map<std::string, image_correspondence_feature> features;
};

image_correspondence_feature decode_image_correspondence_feature(const json&);
json encode_image_correspondence_feature(const image_correspondence_feature&); 

image_correspondences import_image_correspondences_file(const std::string& filename);
void export_image_correspondences_file(const std::string& filename, const image_correspondences&);

}

#endif
