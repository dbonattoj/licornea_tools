#include "view_homography.h"
#include "opencv.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <climits>

namespace tlz {
	
json encode_view_homography(const view_homography& hom) {
	json j_hom = json::object();
	j_hom["mat"] = encode_mat(hom.mat);
	if(! std::isnan(hom.err)) j_hom["err"] = hom.err;
	return j_hom;
}


view_homography decode_view_homography(const json& j_hom) {
	view_homography hom;
	hom.mat = decode_mat(j_hom["mat"]);
	hom.err = get_or(j_hom, "err", NAN);
	return hom;
}


json encode_view_homographies(const view_homographies& homs) {
	json j_homs = json::object();
	for(const auto& kv : homs) {
		const view_index& idx = kv.first;
		const view_homography& hom = kv.second;
		j_homs[encode_view_index(idx)] = encode_view_homography(hom);
	}
	return j_homs;
}


view_homographies decode_view_homographies(const json& j_homs) {
	view_homographies homs;
	for(auto it = j_homs.begin(); it != j_homs.end(); ++it) {
		view_index idx = decode_view_index(it.key());
		const json& j_hom = it.value();
		homs[idx] = decode_view_homography(j_hom);
	}
	return homs;
}


bool is_single_view_homography(const json& j) {
	return has(j, "mat");
}


real view_homographies_error(const view_homographies& homs) {
	real err = 0.0;
	for(const auto& kv : homs) err += kv.second.err;
	err /= homs.size();
	return err;
}


border maximal_border(const view_homography& hom, real width, real height) {
	std::vector<vec2> src {
		vec2(0, 0),
		vec2(width, 0),
		vec2(width, height),
		vec2(0, height)
	};
	
	std::vector<vec2> dst;
	cv::perspectiveTransform(src, dst, hom.mat);
	
	real max_x = std::max({ dst[0][0], dst[1][0], dst[2][0], dst[3][0] });
	real max_y = std::max({ dst[0][1], dst[1][1], dst[2][1], dst[3][1] });
	real min_x = std::min({ dst[0][0], dst[1][0], dst[2][0], dst[3][0] });
	real min_y = std::min({ dst[0][1], dst[1][1], dst[2][1], dst[3][1] });
	
	border bord;
	bord.top = -min_y;
	bord.left = -min_x;
	bord.bottom = max_y - height;
	bord.right = max_x - width;
	return bord;
}


border maximal_border(const view_homographies& homs, real width, real height) {
	border max_bord;
	max_bord.top = INT_MIN;
	max_bord.left = INT_MIN;
	max_bord.bottom = INT_MIN;
	max_bord.right = INT_MIN;
	
	for(const auto& kv : homs) {
		border bord = maximal_border(kv.second, width, height);
		max_bord.top = std::max({ max_bord.top, bord.top });
		max_bord.left = std::max({ max_bord.left, bord.left });
		max_bord.bottom = std::max({ max_bord.bottom, bord.bottom });
		max_bord.right = std::max({ max_bord.right, bord.right });
	}
	
	return max_bord;
}


}
