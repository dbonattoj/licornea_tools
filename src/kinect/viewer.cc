#include "../lib/viewer.h"
#include "lib/live/grabber.h"

using namespace tlz;

int main() {
	grabber grab(grabber::color | grabber::depth | grabber::ir);

	viewer view(754+512+512, 424);
	auto& min_d = view.add_int_slider("depth min ", 0, 0, 20000);
	auto& max_d = view.add_int_slider("depth max", 6000, 0, 20000);
	auto& min_ir = view.add_int_slider("ir min", 0, 0x0000, 0xffff);
	auto& max_ir = view.add_int_slider("ir max", 0xffff, 0x0000, 0xffff);
	
	do {
		grab.grab();
		view.clear();
		
		view.draw(cv::Rect(0, 0, 754, 424), grab.get_color_frame());
		view.draw(cv::Rect(754, 0, 512, 424), grab.get_ir_frame(min_ir.value(), max_ir.value()));
		view.draw_depth(cv::Rect(754+512, 0, 512, 424), grab.get_depth_frame(), min_d.value(), max_d.value());
		
		grab.release();
	} while(view.show());	
}
