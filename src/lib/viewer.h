#ifndef LICORNEA_VIEWER_H_
#define LICORNEA_VIEWER_H_

#include "common.h"
#include "opencv.h"
#include <string>
#include <memory>
#include <vector>
#include <functional>

namespace tlz {

class viewer {
public:
	class slider_base;
	class int_slider;
	class real_slider;

	enum text_alignment {
		left,
		center,
		right
	};
	
private:
	std::string window_name_;
	std::vector<std::unique_ptr<slider_base>> sliders_;
	cv::Mat_<cv::Vec3b> shown_image_;
	bool running_modal_ = false;
	
	static std::string get_window_name_(const std::string& title = std::string());
	
public:
	cv::Vec3b black = cv::Vec3b(0, 0, 0);
	cv::Vec3b white = cv::Vec3b(255, 255, 255);
	cv::Vec3b indicator_color = cv::Vec3b(255, 100, 100);
	cv::Vec3b background_color = cv::Vec3b(0, 0, 0);
	cv::Vec3b text_color = cv::Vec3b(255, 255, 255);

	std::function<void()> update_callback;
	std::function<void(int keycode)> key_callback;
	std::function<void(int event, int x, int y)> mouse_callback;

	viewer(const std::string& title, int width, int height, bool resizeable = false);
	viewer(const std::string& title, cv::Size sz, bool resizeable = false) :
		viewer(title, sz.width, sz.height, resizeable) { }
	viewer(int width, int height, bool resizeable = false);
	viewer(cv::Size sz, bool resizeable = false) :
		viewer(sz.width, sz.height, resizeable) { }
	explicit viewer(const std::string& title, bool resizeable = false);
	
	viewer(const viewer&) = delete;
	~viewer();	
	viewer& operator=(const viewer&) = delete;	
		
	int_slider& add_int_slider(const std::string& caption, int default_val, int min_val, int max_val, int step = 1);
	real_slider& add_real_slider(const std::string& caption, real default_val, real min_val, real max_val, int steps = 100);
	

	int width() const;
	int height() const;
	
	void clear(int width, int height);
	void clear(cv::Size);
	void clear();
	
	void draw(const cv::Mat_<cv::Vec3b>&, real blend = 1.0);
	void draw(const cv::Mat_<uchar>&, real blend = 1.0);
	
	void draw(cv::Rect rect, const cv::Mat_<cv::Vec3b>& img, real blend = 1.0);
	void draw(cv::Point pt, const cv::Mat_<cv::Vec3b>& img, real blend = 1.0)
		{ draw(cv::Rect(pt, img.size()), img, blend); }
		
	void draw(cv::Rect rect, const cv::Mat_<uchar>& img, real blend = 1.0);
	void draw(cv::Point pt, const cv::Mat_<uchar>& img, real blend = 1.0)
		{ draw(cv::Rect(pt, img.size()), img, blend); }
		
	void draw_depth(cv::Rect rect, const cv::Mat_<float>& depth_img, float min_d, float max_d, real blend = 1.0);
	void draw_depth(cv::Point pt, const cv::Mat_<float>& depth_img, float min_d, float max_d, real blend = 1.0)
		{ draw_depth(cv::Rect(pt, depth_img.size()), depth_img, min_d, max_d, blend); }
	
	void draw_text(cv::Rect rect, const std::string& text, text_alignment = left);
	void draw_text(cv::Rect rect, const std::string& text, text_alignment, cv::Vec3b color);
	void draw_2d_cross_indicator(cv::Rect rect, real value_x, real value_y, real max_abs_value);
	void draw_2d_arrow_indicator(cv::Rect rect, real value_x, real value_y, real max_value);

	bool show(int& keycode);
	bool show();
	
	void show_modal();
	void update_modal();
	void close_modal();

	static cv::Mat_<uchar> visualize_depth(const cv::Mat&, float min_d, float max_d);
};


class viewer::slider_base {
public:
	int raw_value;

protected:
	slider_base() = default;
	slider_base(const slider_base&) = delete;
	slider_base(slider_base&&) = delete;
	slider_base& operator=(const slider_base&) = delete;
	slider_base& operator=(slider_base&&) = delete;
};


class viewer::int_slider : public viewer::slider_base {
public:
	const int min;
	const int max;
	const int step;
	
	int_slider(int value_, int min_, int max_, int step_);
	
	int slider_max() const;
	
	int value() const;
	void set_value(int val);
	
	operator int () const { return value(); }
};


class viewer::real_slider : public viewer::slider_base {
public:
	const real min;
	const real max;
	const int steps;
	
	real_slider(real value_, real min_, real max_, int steps_);
	
	int slider_max() const;
	
	real value() const;
	void set_value(real val);
	
	operator real () const { return value(); }
};


}

#endif
