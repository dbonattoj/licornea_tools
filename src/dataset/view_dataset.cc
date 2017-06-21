#include "../lib/args.h"
#include "../lib/opencv.h"
#include "../lib/dataset.h"
#include "../lib/image_io.h"
#include "../lib/viewer.h"
#include <string>

using namespace tlz;


int main(int argc, const char* argv[]) {
	get_args(argc, argv, "dataset_parameters.json [dataset_group]");
	dataset datas = dataset_arg();
	std::string dataset_group_name = string_opt_arg("");
	dataset_group datag = datas.group(dataset_group_name);
	
	viewer view("Dataset Viewer", datag.image_size_with_border(), true);
	auto& slider_x = view.add_int_slider("X", datas.x_mid(), datas.x_min(), datas.x_max(), datas.x_step());
	auto& slider_y = view.add_int_slider("Y", datas.y_mid(), datas.y_min(), datas.y_max(), datas.y_step());

	view.update_callback = [&]() {
		view_index idx(slider_x.value(), slider_y.value());
		if(! datas.valid(idx)) return;		
		
		std::string filename = datag.view(idx).image_filename();
		try {
			cv::Mat_<cv::Vec3b> img = load_texture(filename);
			view.clear(img.cols, img.rows);
			view.draw(img);
		} catch(const std::runtime_error&) {
			std::cout << "could not load " << filename << std::endl; 
			view.clear();
		}
	};

	view.show_modal();
}
