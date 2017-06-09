#include "viewer.h"

namespace tlz {

namespace {
	int viewers_count_ = 0;
	
	void slider_callback_(int, void* userdata) {	
		viewer& vw = *static_cast<viewer*>(userdata);
		vw.update_modal();
	}

	void mouse_callback_(int event, int x, int y, int, void* userdata) {
		viewer& vw = *static_cast<viewer*>(userdata);
		if(vw.mouse_callback)
			vw.mouse_callback(event, x, y); // TODO window size?
	}
}


std::string viewer::get_window_name_(const std::string& req_title) {
	std::string title;
	if(req_title.empty()) title = "Viewer";
	else title = req_title;
	if(viewers_count_ > 0) title += " (" + std::to_string(viewers_count_ + 1) + ")";
	viewers_count_++;
	return title;
}

viewer::viewer(const std::string& title, int w, int h, bool resizeable) :
	window_name_(get_window_name_(title)),
	shown_image_(h, w)
{	
	if(resizeable) {
		cv::namedWindow(window_name_, CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED);
		cv::resizeWindow(window_name_, w, h);
	} else {
		cv::namedWindow(window_name_, CV_WINDOW_AUTOSIZE | CV_GUI_EXPANDED);
	}
	
	cv::setMouseCallback(window_name_, mouse_callback_, this);
}


viewer::viewer(int w, int h, bool resizeable) :
	viewer("", w, h, resizeable) { }


viewer::viewer(const std::string& title, bool resizeable) :
	window_name_(get_window_name_(title))
{
	if(resizeable)
		cv::namedWindow(window_name_, CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED);
	else
		cv::namedWindow(window_name_, CV_WINDOW_AUTOSIZE | CV_GUI_EXPANDED);
		
	cv::setMouseCallback(window_name_, mouse_callback_, this);
}


viewer::~viewer() {
	cv::destroyWindow(window_name_);
}


int viewer::width() const {
	return shown_image_.cols;
}


int viewer::height() const {
	return shown_image_.rows;
}


void viewer::clear(int width, int height) {
	if(width != shown_image_.cols || height != shown_image_.rows) {
		shown_image_ = cv::Mat_<cv::Vec3b>(height, width, background_color);
		cv::resizeWindow(window_name_, width, height);
	} else {
		shown_image_.setTo(background_color);
	}
}

void viewer::clear(cv::Size sz) {
	clear(sz.width, sz.height);
}


void viewer::clear() {
	shown_image_.setTo(background_color);
}


cv::Mat_<uchar> viewer::visualize_depth(const cv::Mat& depth_img, float min_d, float max_d) {
	cv::Mat_<uchar> viz_depth_img;
	float alpha = 255.0f / (max_d - min_d);
	float beta = -alpha * min_d;
	cv::convertScaleAbs(depth_img, viz_depth_img, alpha, beta);
	viz_depth_img.setTo(0, (depth_img < min_d));
	viz_depth_img.setTo(255, (depth_img > max_d));
	viz_depth_img.setTo(0, (depth_img == 0));
	return viz_depth_img;
}


void viewer::draw(const cv::Mat_<cv::Vec3b>& img, real blend) {
	draw(cv::Rect(0, 0, width(), height()), img, blend);
}


void viewer::draw(const cv::Mat_<uchar>& img, real blend) {
	draw(cv::Rect(0, 0, width(), height()), img, blend);
}



void viewer::draw(cv::Rect rect, const cv::Mat_<cv::Vec3b>& img, real blend) {
	cv::Mat resized_img;
	float ratio = (float)img.rows / img.cols;
	float h1 = rect.width * ratio;
	float w2 = rect.height / ratio;
	if(h1 < rect.height) cv::resize(img, resized_img, cv::Size(rect.width, h1));
	else cv::resize(img, resized_img, cv::Size(w2, rect.height));
	int top = (rect.height - resized_img.rows) / 2;
	int down = (rect.height - resized_img.rows + 1) / 2;
	int left = (rect.width - resized_img.cols) / 2;
	int right = (rect.width - resized_img.cols + 1) / 2;
	cv::copyMakeBorder(resized_img, resized_img, top, down, left, right, cv::BORDER_CONSTANT, cv::Scalar(background_color));
	
	if(blend < 1.0) {
		cv::Mat underlying_image(shown_image_, rect);
		cv::Mat sum;
		cv::addWeighted(resized_img, blend, underlying_image, 1.0-blend, 0.0, sum);
		sum.copyTo(cv::Mat(shown_image_, rect));
	} else {
		resized_img.copyTo(cv::Mat(shown_image_, rect));
	}
}


void viewer::draw(cv::Rect rect, const cv::Mat_<uchar>& img, real blend) {
	cv::Mat_<cv::Vec3b> conv_img;
	cv::cvtColor(img, conv_img, CV_GRAY2BGR);
	draw(rect, conv_img, blend);
}


void viewer::draw_depth(cv::Rect rect, const cv::Mat_<float>& depth_img, float min_d, float max_d, real blend) {
	cv::Mat_<uchar> viz_depth_img = visualize_depth(depth_img, min_d, max_d);
	draw(rect, viz_depth_img, blend);
}


void viewer::draw_text(cv::Rect rect, const std::string& text, text_alignment align, cv::Vec3b color) {
	int font = cv::FONT_HERSHEY_COMPLEX_SMALL;
	double fontscale = 0.8;
	int thickness = 1;

	cv::Size sz = cv::getTextSize(text, font, fontscale, thickness, nullptr);
	
	int y = rect.y + sz.height + (rect.height/2 - sz.height/2);
	int x;
	if(align == left) x = rect.x;
	else if(align == center) x = rect.x + (rect.width/2 - sz.width/2);
	else x = rect.x + rect.width - sz.width;
		
	cv::putText(shown_image_, text, cv::Point(x, y), font, fontscale, cv::Scalar(color), thickness);
}
void viewer::draw_text(cv::Rect rect, const std::string& text, text_alignment align) {
	draw_text(rect, text, align, text_color);
}

void viewer::draw_2d_cross_indicator(cv::Rect rect, real value_x, real value_y, real max_abs_value) {
	std::vector<std::vector<cv::Point>> polylines;
	polylines.push_back({ cv::Point(rect.x+rect.width/2, rect.y), cv::Point(rect.x+rect.width/2, rect.y+rect.height) });
	polylines.push_back({ cv::Point(rect.x, rect.y+rect.height/2), cv::Point(rect.x+rect.width, rect.y+rect.height/2) });
	cv::polylines(shown_image_, polylines, false, cv::Scalar(black), 3);
	cv::polylines(shown_image_, polylines, false, cv::Scalar(white), 1);
	
	
	const int indicator_rad = 20;
	int indicator_x = (rect.x+rect.width/2) + value_x * 0.5 * (rect.width / max_abs_value);
	int indicator_y = (rect.y+rect.height/2) + value_y * 0.5 * (rect.height / max_abs_value);
	polylines.clear();
	polylines.push_back({ cv::Point(indicator_x-indicator_rad, indicator_y), cv::Point(indicator_x+indicator_rad, indicator_y) });
	polylines.push_back({ cv::Point(indicator_x, indicator_y-indicator_rad), cv::Point(indicator_x, indicator_y+indicator_rad) });
	
	cv::polylines(shown_image_, polylines, false, cv::Scalar(white), 5);
	cv::polylines(shown_image_, polylines, false, cv::Scalar(indicator_color), 3);
}


void viewer::draw_2d_arrow_indicator(cv::Rect rect, real value_x, real value_y, real max_value) {
	int indicator_x = (rect.x) + value_x * (rect.width / max_value);
	int indicator_y = (rect.y+rect.height) - value_y * (rect.height / max_value);

	cv::Point pt1(rect.x, rect.y+rect.height);
	cv::Point pt2(indicator_x, indicator_y);
	cv::line(shown_image_, pt1, pt2, cv::Scalar(indicator_color), 3);
	cv::circle(shown_image_, pt2, 4, cv::Scalar(indicator_color), -1);
}



bool viewer::show(int& keycode) {
	cv::imshow(window_name_, shown_image_);
	keycode = cv::waitKey(1);
	if(keycode == escape_keycode) return false;
	else return true;
}
bool viewer::show() {
	int unused;
	return show(unused);
}


void viewer::show_modal() {
	running_modal_ = true;
	try {
		int keycode = 0;
		while(running_modal_) {
			update_modal();
			keycode = cv::waitKey(0);

			if(keycode == escape_keycode) running_modal_ = false;
			else if(keycode > 0 && key_callback) key_callback(keycode);

			void* handle = cvGetWindowHandle(window_name_.c_str());
			if(handle == nullptr) running_modal_ = false;
		}
	} catch(...) {
		running_modal_ = false;
		throw;
	}
}


void viewer::update_modal() {
	if(! running_modal_) return;
	if(update_callback) update_callback();
	cv::imshow(window_name_, shown_image_);
}


void viewer::close_modal() {
	running_modal_ = false;
}


viewer::int_slider& viewer::add_int_slider(const std::string& caption, int default_val, int min_val, int max_val, int step) {
	auto slid_ptr = std::make_unique<int_slider>(default_val, min_val, max_val, step);
	int_slider& slid = *slid_ptr;
	sliders_.emplace_back(std::move(slid_ptr));
	cv::createTrackbar(caption, window_name_, &slid.raw_value, slid.slider_max(), &slider_callback_, this);
	return slid;
}


viewer::real_slider& viewer::add_real_slider(const std::string& caption, real default_val, real min_val, real max_val, int steps) {
	auto slid_ptr = std::make_unique<real_slider>(default_val, min_val, max_val, steps);
	real_slider& slid = *slid_ptr;
	sliders_.emplace_back(std::move(slid_ptr));
	cv::createTrackbar(caption, window_name_, &slid.raw_value, slid.slider_max(), &slider_callback_, this);
	return slid;
}




//////////


viewer::int_slider::int_slider(int value_, int min_, int max_, int step_) :
	min(min_), max(max_), step(step_) { set_value(value_); }

int viewer::int_slider::slider_max() const {
	return (max - min) / step;
}

int viewer::int_slider::value() const {
	return min + step*raw_value;
}

void viewer::int_slider::set_value(int val) {
	raw_value = (val - min)/step;
}



viewer::real_slider::real_slider(real value_, real min_, real max_, int steps_) :
	min(min_), max(max_), steps(steps_) { set_value(value_); }

int viewer::real_slider::slider_max() const {
	return steps;
}

real viewer::real_slider::value() const {
	real range = max - min;
	return min + raw_value*range/steps;
}

void viewer::real_slider::set_value(real val) {
	real range = max - min;
	raw_value = steps * (val - min)/range;
}


	
}
