#include "../lib/common.h"
#include "../lib/args.h"
#include "../lib/view_homography.h"
#include "../lib/border.h"

using namespace tlz;

int main(int argc, const char* argv[]) {
	get_args(argc, argv, "homography.json/homographies.json width height maximal_border out_border.json");
	json j = json_arg();
	int width = int_arg();
	int height = int_arg();
	std::string out_border_filename = out_filename_arg();
	
	border bord;
	if(is_single_view_homography(j)) {
		view_homography	hom = decode_view_homography(j);
		bord = maximal_border(hom, width, height);
	} else {
		view_homographies homs = decode_view_homographies(j);
		bord = maximal_border(homs, width, height);
	}
	
	export_json_file(encode_border(bord), out_border_filename);
}
