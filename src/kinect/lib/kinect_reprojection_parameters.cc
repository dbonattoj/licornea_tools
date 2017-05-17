#include "kinect_reprojection_parameters.h"

namespace tlz {

json encode_kinect_reprojection_parameters(const kinect_reprojection_parameters& parameters) {
	json j_parameters = json::object();
	j_parameters["ir_intrinsics"] = encode_intrinsics(parameters.ir_intrinsics);
	j_parameters["color_intrinsics"] = encode_intrinsics(parameters.color_intrinsics);
	j_parameters["rotation"] = encode_mat(parameters.rotation);
	j_parameters["translation"] = encode_mat(parameters.translation);
	return j_parameters;
}


kinect_reprojection_parameters decode_kinect_reprojection_parameters(const json& j_parameters) {
	kinect_reprojection_parameters parameters;
	parameters.ir_intrinsics = decode_intrinsics(j_parameters["ir_intrinsics"]);
	parameters.color_intrinsics = decode_intrinsics(j_parameters["color_intrinsics"]);
	parameters.rotation = decode_mat(j_parameters["rotation"]);
	parameters.translation = decode_mat(j_parameters["translation"]);
	return parameters;
}

}

