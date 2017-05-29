#include "../lib/args.h"
#include "../lib/json.h"

using namespace tlz;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "in.json out.json");
	std::string in_filename = in_filename_arg();
	std::string out_filename = out_filename_arg();

	json j = import_json_file(in_filename);
	export_json_file(j, out_filename);
}
