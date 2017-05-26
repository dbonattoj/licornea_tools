#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include "../lib/camera.h"

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: visualize in_cameras.json out_view.ply world/view\n";
	std::cout << std::endl;
	std::exit(1);
}

int main(int argc, const char* argv[]) {
	if(argc <= 3) usage_fail();
	std::string in_cameras = argv[1];
	std::string out_ply = argv[2];
	std::string mode = argv[3];
	if(mode != "world" && mode != "view") usage_fail();
	bool world = (mode == "world");
	
	std::ifstream input(in_cameras.c_str());
	input.exceptions(std::ios_base::badbit);	
	
	auto cameras = read_cameras_file(in_cameras);
	if(cameras.size() == 0) {
		std::cout << "no cameras in file" << std::endl;
		usage_fail();
	} else {
		std::cout << cameras.size() << " cameras..." << std::endl;
	}
			
	real scale = 1.0;
	mat44 model_transform(
		2.0*scale, 0.0, 0.0, 0.0,
		0.0, 1.0*scale, 0.0, 0.0,
		0.0, 0.0, 1.0*scale, 0.0,
		0.0, 0.0, 0.0, 1.0
	);
	
	std::vector<vec3> vertices;
	std::vector<std::vector<std::ptrdiff_t>> faces;
	
	if(world) {
		real camera_depth = 2.0;
		vertices = {
			vec3(0, 0, 0),    // 0: base
			vec3(+1, -1, camera_depth),  // 1: top left
			vec3(+1, +1, camera_depth),  // 2: top right
			vec3(-1, +1, camera_depth),  // 3: bottom left
			vec3(-1, -1, camera_depth)   // 4: bottom right
		};
		faces = {
			{0, 2, 1}, // top
			{0, 3, 2}, // right
			{0, 4, 3}, // bottom,
			{0, 1, 4}, // left,
			{1, 2, 3, 4}, // back
		};
		
	} else {
		vec3 sz(1.0, 1.0, 0.5);
		vertices = {
			vec3(-sz[0], +sz[1], +sz[2]),
			vec3(+sz[0], +sz[1], +sz[2]),
			vec3(-sz[0], -sz[1], +sz[2]),
			vec3(+sz[0], -sz[1], +sz[2]),
			vec3(-sz[0], +sz[1], -sz[2]),
			vec3(+sz[0], +sz[1], -sz[2]),
			vec3(-sz[0], -sz[1], -sz[2]),
			vec3(+sz[0], -sz[1], -sz[2])
		};
		faces = {
			{0, 2, 3, 1}, // front
			{4, 5, 7, 6}, // back
			{4, 6, 2, 0}, // left
			{1, 3, 7, 5}, // right
			{4, 0, 1, 5}, // top
			{2, 6, 7, 3}  // bottom
		};
	}

	
	std::ofstream output(out_ply.c_str());
	output << "ply\n";
	output << "format ascii 1.0\n";
	output << "element vertex " << cameras.size() * vertices.size() << '\n';
	output << "property float x\n";
	output << "property float y\n";
	output << "property float z\n";
	output << "element face " << cameras.size() * faces.size() << '\n';
	output << "property list uchar int vertex_indices\n";
	output << "end_header\n";
	
	for(const camera& cam : cameras) {	
		mat44 M;
		if(world) M = cam.extrinsic().inv() * model_transform;
		else M = cam.extrinsic() * model_transform;
			
		for(const vec3& model_vertex : vertices) {
			vec3 shown_vertex = mul_h(M, model_vertex);
			output << shown_vertex[0] << ' ' << shown_vertex[1] << ' ' << shown_vertex[2] << '\n';
		}
	}
	
	int idx = 0;
	for(std::ptrdiff_t camera_index = 0; camera_index < cameras.size(); ++camera_index) {
		for(const std::vector<std::ptrdiff_t>& face : faces) {
			output << face.size();
			for(std::ptrdiff_t face_idx : face) output << ' ' << face_idx + idx;
			output << '\n';
		}
		idx += vertices.size();
	}
	
	std::cout << "done" << std::endl;
}
