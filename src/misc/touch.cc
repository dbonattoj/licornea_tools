#include "../lib/common.h"
#include "../lib/args.h"
#include <fstream>

using namespace tlz;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "out.file [no_create]");
	std::string out_filename = out_filename_arg();
	bool no_create = bool_opt_arg("no_create");
	
	if(! no_create) {
		std::ofstream str(out_filename);
		str.close();
	}
}

