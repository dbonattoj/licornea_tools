#include "viewer.h"

namespace tlz {

const int viewer::enter_keycode = 10;
const int viewer::escape_keycode = 27;

int viewer::viewers_count_ = 0;

viewer::viewer(int w, int h, bool resizeable) :
	window_name_("Viewer" + (viewers_count_ > 0 ? std::string(" (") + std::to_string(viewers_count_+1) + ")" : std::string())),
	shown_image_(h, w)
{
	viewers_count_++;
	if(resizeable) cv::namedWindow(window_name_, CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED);
	else cv::namedWindow(window_name_, CV_WINDOW_AUTOSIZE | CV_GUI_EXPANDED);
}


viewer::~viewer() {
	cv::destroyWindow(window_name_);
}


viewer::slider& viewer::add_slider(const std::string& caption, int default_val, int max_val) {
	sliders_.emplace_back(std::make_unique<slider>());
	slider& slid = *sliders_.back();
	slid.value = default_val;
	cv::createTrackbar(caption, window_name_, &slid.value, max_val);
	return slid;
}


void viewer::clear() {
	shown_image_.setTo(background_color);
}


cv::Mat_<uchar> viewer::visualize_depth(const cv::Mat_<float>& depth_img, float min_d, float max_d) {
	cv::Mat_<uchar> viz_depth_img;
	float alpha = 255.0f / (max_d - min_d);
	float beta = -alpha * min_d;
	cv::convertScaleAbs(depth_img, viz_depth_img, alpha, beta);
	viz_depth_img.setTo(0, (depth_img < min_d));
	viz_depth_img.setTo(255, (depth_img > max_d));
	viz_depth_img.setTo(0, (depth_img == 0));
	return viz_depth_img;
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

	
}
