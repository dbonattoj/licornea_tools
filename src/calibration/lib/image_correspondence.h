#ifndef LICORNEA_IMAGE_CORRESPONDENCE_H_
#define LICORNEA_IMAGE_CORRESPONDENCE_H_

#include "../../lib/json.h"
#include "../../lib/dataset.h"
#include "../../lib/args.h"
#include "../../lib/intrinsics.h"
#include "feature_point.h"
#include <map>
#include <vector>
#include <utility>
#include <set>

namespace tlz {


/// Feature on set of views. Optionally one view is "reference".
struct image_correspondence_feature {
	view_index reference_view;
	std::map<view_index, feature_point> points;
	// when iterating (--> encoding into json array), gets sorted by index
	
	bool has(view_index idx) const
		{ return (points.find(idx) != points.end()); }
};

/// Set of features, each on set of views.
struct image_correspondences {
	std::string dataset_group;
	std::map<std::string, image_correspondence_feature> features;
	
	bool has_feature(const std::string& feature_name) const
		{ return (features.find(feature_name) != features.end()); }
};

image_correspondence_feature decode_image_correspondence_feature(const json&);
json encode_image_correspondence_feature(const image_correspondence_feature&); 

image_correspondences decode_image_correspondences(const json&);
json encode_image_correspondences(const image_correspondences&);

void export_binary_image_correspondences(const image_correspondences& cors, const std::string& filename);
image_correspondences import_binary_image_correspondences(const std::string& filename);

void export_image_corresponcences(const image_correspondences& cors, const std::string& filename);
image_correspondences import_image_correspondences(const std::string& filename);

std::set<view_index> reference_views(const image_correspondences&);
image_correspondences image_correspondences_with_reference(const image_correspondences&, const view_index& reference_view);

std::set<view_index> all_views(const image_correspondences&);

std::set<std::string> feature_names(const image_correspondences&);

image_correspondences undistort(const image_correspondences&, const intrinsics&);

cv::Mat_<cv::Vec3b> visualize_view_points(const image_correspondence_feature&, const cv::Mat_<cv::Vec3b>& back_img, const cv::Vec3b& col, int dot_size = 2, const border& = border());

image_correspondences image_correspondences_arg();

}

#endif
