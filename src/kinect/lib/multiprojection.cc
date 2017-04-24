#include "multiprojection.h"

namespace tlz {

multiprojection decode_multiprojection(const json& j) {
	multiprojection mproj;
	mproj.out_camera_name = j["out_camera_name"];
	for(const json& j_in : j["inputs"]) {
		multiprojection::input in;
		in.camera_name = j_in["camera_name"];
		in.point_cloud_filename = j_in["point_cloud_filename"];		
		mproj.inputs.push_back(in);
	}
	return mproj;
}


json encode_multiprojection(const multiprojection& mproj) {
	json j = json::object();
	j["out_camera_name"] = mproj.out_camera_name;
	j["inputs"] = json::array();
	for(const multiprojection::input& in : mproj.inputs) {
		json j_in = json::object();
		j_in["camera_name"] = in.camera_name;
		j_in["point_cloud_filename"] = in.point_cloud_filename;
		j["inputs"].push_back(j_in);
	}
	return j;
}


}
