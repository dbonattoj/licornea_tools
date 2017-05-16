#include "lib/live/viewer.h"
#include "lib/live/grabber.h"

using namespace tlz;

int main() {
	grabber grab(grabber::color | grabber::ir);

	viewer view(754+512, 424);
	
	for(;;) {
		grab.grab();
		view.clear();
		
		view.draw(cv::Rect(0, 0, 754, 424), grab.get_color_frame());
		view.draw(cv::Rect(754, 0, 512, 424), grab.get_ir_frame());
		
		grab.release();
		view.show();
	}
}
