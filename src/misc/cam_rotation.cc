#include "../lib/args.h"
#include "../lib/json.h"
#include "../lib/rotation.h"
#include <iostream>
#include <cstdlib>
#include <cmath>

using namespace tlz;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "(from rotation_matrix.json / to out_rotation_matrix.json x y z)");
	std::string mode = enum_arg({"from", "to"});
	
	if(mode == "from") {
		std::string rotation_mat_filename = in_filename_arg();
				
		mat33 R = decode_mat(import_json_file(rotation_mat_filename));
		
		vec3 euler = to_euler(R);
				
		std::cout << "x = " << euler[0] * deg_per_rad << "°" << std::endl;
		std::cout << "y = " << euler[1] * deg_per_rad << "°" << std::endl;
		std::cout << "z = " << euler[2] * deg_per_rad << "°" << std::endl;
		
	} else if(mode == "to") {
		std::string out_rotation_mat_filename = out_filename_arg();
		real x = real_arg() * rad_per_deg;
		real y = real_arg() * rad_per_deg;
		real z = real_arg() * rad_per_deg;
		
		vec3 euler(x, y, z);
		mat33 R = to_rotation_matrix(euler);
		
		export_json_file(encode_mat(R), out_rotation_mat_filename);
	}	
}
