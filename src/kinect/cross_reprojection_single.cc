#include "../lib/args.h"
#include "../lib/camera.h"
#include "../lib/dataset.h"

using namespace tlz;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json cameras.json x,y source_x,source_y");
	dataset datas = dataset_arg();
	camera_array cams = cameras_arg();
	view_index idx = view_index_arg();
	view_index source_idx = view_index_arg();
	
	
}
