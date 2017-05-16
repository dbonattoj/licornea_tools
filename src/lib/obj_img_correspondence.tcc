namespace tlz {

template<std::size_t Obj_count, std::size_t Img_count>
obj_img_correspondence<Obj_count, Img_count> decode_obj_img_correspondence(const json& j_cor) {
	obj_img_correspondence<Obj_count, Img_count> cor;
	for(int i = 0; i < Obj_count; ++i)
		cor.object_coordinates[i] = decode_mat(j_cor["object" + (Obj_count > 1 ? std::to_string(i+1) : std::string())]);
	for(int i = 0; i < Img_count; ++i)
		cor.image_coordinates[i] = decode_mat(j_cor["image" + (Img_count > 1 ? std::to_string(i+1) : std::string())]);
	return cor;
}

template<std::size_t Obj_count, std::size_t Img_count>
json encode_obj_img_correspondence(const obj_img_correspondence<Obj_count, Img_count>& cor) {
	json j_cor = json::object();
	for(int i = 0; i < Obj_count; ++i)
		j_cor["object" + (Obj_count > 1 ? std::to_string(i+1) : std::string())] = encode_mat(cor.object_coordinates[i]);
	for(int i = 0; i < Img_count; ++i)
		j_cor["image" + (Img_count > 1 ? std::to_string(i+1) : std::string())] = encode_mat(cor.image_coordinates[i]);
	return j_cor;
}


template<std::size_t Obj_count, std::size_t Img_count>
json encode_obj_img_correspondences_set(const obj_img_correspondences_set<Obj_count, Img_count>& set) {
	json j_set = json::array();
	for(const auto& cors : set) {
		json j_cors = json::array();
		for(const auto& cor : cors)
			j_cors.push_back(encode_obj_img_correspondence(cor));
		j_set.push_back(j_cors);
	}
	return j_set;
}


template<std::size_t Obj_count, std::size_t Img_count>
obj_img_correspondences_set<Obj_count, Img_count> decode_obj_img_correspondences_set(const json& j_set) {
	obj_img_correspondences_set<Obj_count, Img_count> set;
	for(const auto& j_cors : j_set) {
		obj_img_correspondences<Obj_count, Img_count> cors;
		for(const auto& j_cor : j_cors)
			cors.push_back(decode_obj_img_correspondence<Obj_count, Img_count>(j_cor));
		set.push_back(cors);
	}
	return set;
}

}
