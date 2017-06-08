#include "../lib/args.h"
#include "../lib/opencv.h"
#include "../lib/dataset.h"
#include "../lib/image_io.h"
#include "../lib/viewer.h"

using namespace tlz;


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json [dataset_group]");
	dataset datas = dataset_arg();
	std::string dataset_group = string_opt_arg("");
	dataset_group datag = datas.group(dataset_group);
	
	viewer view("Dataset Viewer", true);
	auto& slider_x = view.add_int_slider("X", datas.x_mid(), datas.x_min(), datas.x_max(), datas.x_step());
	auto& slider_y = view.add_int_slider("Y", datas.y_mid(), datas.y_min(), datas.y_max(), datas.y_step());

	view.update_callback = [&]() {
		view_index idx(slider_x.value(), slider_y.value());
		if(! datas.valid(idx)) return;		
		
		cv::Mat_<cv::Vec3b> img = load_texture(datag.view(idx).image_filename());
		view.clear(img.cols, img.rows);
		view.draw(img);
	};

	view.show_modal();
}
