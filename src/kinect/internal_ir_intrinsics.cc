#include "lib/args.h"
#include "lib/kinect_internal_parameters.h"
#include "../lib/intrinsics.h"
#include "../lib/json.h"
#include <iostream>
#include <cstdlib>

using namespace tlz;


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "internal_parameters.json out_ir_intr.json");
	std::string internal_parameters_filename = in_filename_arg();
	std::string ir_intrinsics_filename = out_filename_arg();

	kinect_internal_parameters internal_parameters = decode_kinect_internal_parameters(import_json_file(internal_parameters_filename));
	const auto& par = internal_parameters.ir;
	
	intrinsics ir_intr;
	
	ir_intr.K = mat33(
		par.fx, 0.0, par.cx,
		0.0, par.fy, par.cy,
		0.0, 0.0, 1.0
	);
	ir_intr.distortion.k1 = par.k1;
	ir_intr.distortion.k2 = par.k2;
	ir_intr.distortion.k3 = par.k3;
	ir_intr.distortion.p1 = par.p1;
	ir_intr.distortion.p2 = par.p2;
	ir_intr.width = 512;
	ir_intr.height = 424;

	export_json_file(encode_intrinsics(ir_intr), ir_intrinsics_filename);
}
