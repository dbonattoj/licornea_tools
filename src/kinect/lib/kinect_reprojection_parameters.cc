#include "kinect_reprojection_parameters.h"

namespace tlz {

namespace {

json encode_depth_offset_(const kinect_reprojection_parameters::depth_offset_polyfit& fit) {
	json j_fit = json::object();
	j_fit["x0y0"] = fit.x0y0;
	j_fit["x1y0"] = fit.x1y0;
	j_fit["x0y1"] = fit.x0y1;
	return j_fit;
}

kinect_reprojection_parameters::depth_offset_polyfit decode_depth_offset_(const json& j_fit) {
	kinect_reprojection_parameters::depth_offset_polyfit fit;
	fit.x0y0 = get_or(j_fit, "x0y0", 0.0);
	fit.x1y0 = get_or(j_fit, "x1y0", 0.0);
	fit.x0y1 = get_or(j_fit, "x0y1", 0.0);
	
	std::cout << fit.x0y0 << std::endl;
	return fit;
}

}

json encode_kinect_reprojection_parameters(const kinect_reprojection_parameters& parameters) {
	json j_parameters = json::object();
	j_parameters["ir_intrinsics"] = encode_intrinsics(parameters.ir_intrinsics);
	j_parameters["color_intrinsics"] = encode_intrinsics(parameters.color_intrinsics);
	j_parameters["rotation"] = encode_mat(parameters.rotation);
	j_parameters["translation"] = encode_mat(parameters.translation);
	j_parameters["depth_offset"] = encode_depth_offset_(parameters.depth_offset);
	return j_parameters;
}


kinect_reprojection_parameters decode_kinect_reprojection_parameters(const json& j_parameters) {
	kinect_reprojection_parameters parameters;
	parameters.ir_intrinsics = decode_intrinsics(j_parameters["ir_intrinsics"]);
	parameters.color_intrinsics = decode_intrinsics(j_parameters["color_intrinsics"]);
	parameters.rotation = decode_mat(j_parameters["rotation"]);
	parameters.translation = decode_mat(j_parameters["translation"]);
	if(has(j_parameters, "depth_offset")) parameters.depth_offset = decode_depth_offset_(j_parameters["depth_offset"]);
	return parameters;
}

}

