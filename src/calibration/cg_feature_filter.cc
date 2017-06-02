#include "../lib/args.h"

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json cors.json out_cors.json");
}
