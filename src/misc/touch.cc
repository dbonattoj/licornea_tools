#include "../lib/common.h"
#include "../lib/args.h"
#include <fstream>

using namespace tlz;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "out.file");
	std::string out_filename = out_filename_arg();
	
	std::ofstream str(out_filename);
	str.close();
}

