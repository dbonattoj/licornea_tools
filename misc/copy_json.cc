#include "../lib/json.h"
#include <iostream>
#include <cstdlib>

using namespace tlz;

[[noreturn]] void usage_fail() {
	std::cout << "usage: copy_json in.json out.json" << std::endl;
	std::exit(1);
}

int main(int argc, const char* argv[]) {
	if(argc <= 2) usage_fail();
	std::string in_filename = argv[1];
	std::string out_filename = argv[2];
	
	json j = import_json_file(in_filename);
	export_json_file(j, out_filename);
}
