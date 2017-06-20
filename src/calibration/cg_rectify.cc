#include "../lib/common.h"
#include "../lib/args.h"
#include "../lib/json.h"
#include "../lib/opencv.h"
#include "../lib/camera.h"
#include "../lib/dataset.h"
#include "../lib/intrinsics.h"
#include "../lib/misc.h"
#include "../lib/assert.h"
#include "../lib/view_homography.h"
#include "lib/feature_points.h"
#include "lib/image_correspondence.h"
#include <map>
#include <iostream>
#include <vector>
#include <cmath>
#include <utility>
#include <set>

using namespace tlz;

const bool verbose = false;


mat33 horizontal_unrotate_out_R(const mat33& R) {
	vec3 n(1, 0, 0); // normal to plane x=0
	vec3 d = R * vec3(0, 0, 1);
	vec3 yd = d - (n * d.dot(n));
	yd = yd / std::sqrt(sq(yd[1]) + sq(yd[2]));	
	mat33 Rx(
		1.0, 0.0, 0.0,
		0.0, yd[2], -yd[1],
		0.0, yd[1], yd[2]
	);
	return Rx.t();
}

mat33 vertical_unrotate_out_R(const mat33& R) {
	vec3 n(0, 1, 0); // normal to plane y=0
	vec3 d = R * vec3(0, 0, 1);
	vec3 xd = d - (n * d.dot(n));
	xd = xd / std::sqrt(sq(xd[0]) + sq(xd[2]));
	mat33 Ry(
		xd[2], 0.0, xd[0],
		0.0, 1.0, 0.0,
		-xd[0], 0.0, xd[2]
	);
	return Ry.t();
}


mat33 homography_matrix(const mat33& R, const vec3& t, real d, const intrinsics& intr) {
	real fx = intr.fx(), fy = intr.fy(), cx = intr.cx(), cy = intr.cy();
	real r11 = R(0,0), r12 = R(0,1), r13 = R(0,2);
	real r21 = R(1,0), r22 = R(1,1), r23 = R(1,2);
	real r31 = R(2,0), r32 = R(2,1), r33 = R(2,2);
	real t1 = t[0], t2 = t[1], t3 = t[2];
	mat33 H;

	H(0,0) = d * (r11 + cx*r31/fx);
	H(0,1) = d * (fx*r12 + cx*r32)/fy;
	H(0,2) = -((cx*cx*d*r31)/fx)+fx*(-((cy*d*r12)/fy)+d*r13+t1)+cx*(d*(-r11-(cy*r32)/fy+r33)+t3);
	
	H(1,0) = d * (fy*r21 + cy*r31)/fx;
	H(1,1) = d * (r22 + cy*r32/fy);
	H(1,2) = -((cx*d*(fy*r21+cy*r31))/fx)-(cy*cy*d*r32)/fy+fy*(d*r23+t2)+cy*(-(d*r22)+d*r33+t3);
	
	H(2,0) = d * r31/fx;
	H(2,1) = d * r32/fy;
	H(2,2) = d*(-((cx*r31)/fx)-(cy*r32)/fy+r33)+t3;
	
	return H;
}


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json intrinsics.json cameras.json out_cameras.json out_homographies.json horizontal/vertical/both aligned_feature_distance");
	dataset datas = dataset_arg();
	intrinsics intr = intrinsics_arg();
	camera_array cameras = cameras_arg();
	std::string out_cameras_filename = out_filename_arg();
	std::string out_homographies_filename = out_filename_arg();
	std::string mode = enum_arg({ "horizontal", "vertical", "both" });
	real aligned_feature_distance = real_arg();
	
	auto cams_map = cameras_map(cameras);
		
	std::vector<real> average_x(datas.x_max()+1, 0.0);
	std::vector<real> average_y(datas.y_max()+1, 0.0);
	for(const view_index& idx : datas.indices()) {
		std::string camera_name = datas.view(idx).camera_name();
		auto cam_it = cams_map.find(camera_name);
		if(cam_it == cams_map.end()) continue;
		const camera& cam = cam_it->second;

		vec3 cam_position = cam.rotation.t() * cam.translation;
		average_x[idx.x] += cam_position[0];
		average_y[idx.y] += cam_position[1];
	}
	for(int x : datas.x_indices()) average_x[x] /= datas.y_count();
	for(int y : datas.y_indices()) average_y[y] /= datas.x_count();
	
	camera_array out_cameras;
	view_homographies out_homographies;
		
	for(const view_index& idx : datas.indices()) {
		std::string camera_name = datas.view(idx).camera_name();
		auto cam_it = cams_map.find(camera_name);
		if(cam_it == cams_map.end()) continue;
		const camera& cam = cam_it->second;
				
		vec3 cam_position = cam.rotation.t() * cam.translation;
		
		vec3 out_cam_position = cam_position;
		if(mode == "horizontal" || mode == "both") out_cam_position[1] = average_y[idx.y];
		if(mode == "vertical" || mode == "both") out_cam_position[0] = average_x[idx.x];

		mat33 out_R;
		if(mode == "horizontal") out_R = horizontal_unrotate_out_R(cam.rotation);
		else if(mode == "vertical") out_R = vertical_unrotate_out_R(cam.rotation);
		else out_R = mat33::eye();
		
		camera out_cam = cam;
		out_cam.rotation = out_R;
		out_cam.translation = out_R * out_cam_position;
		out_cameras.push_back(out_cam);
		
		mat33 relative_rotation = out_R * cam.rotation.t();
		vec3 relative_translation = -relative_rotation * cam.translation + out_cam.translation;
		mat33 homography = homography_matrix(relative_rotation, relative_translation, aligned_feature_distance, intr);
		out_homographies[idx].mat = homography;
	}
	
	export_cameras_file(out_cameras, out_cameras_filename);
	export_json_file(encode_view_homographies(out_homographies), out_homographies_filename);
}

