#ifndef LICORNEA_KINECT_LIVE_VIEWER_H_
#define LICORNEA_KINECT_LIVE_VIEWER_H_

#include "../../lib/common.h"
#include "../../lib/opencv.h"
#include <string>
#include <memory>
#include <vector>

namespace tlz {

class viewer {
public:
	struct slider {
		int value;
		
		slider() = default;
		slider(const slider&) = delete;
		slider(slider&&) = delete;
		slider& operator=(const slider&) = delete;
		slider& operator=(slider&&) = delete;
	};

	enum text_alignment {
		left,
		center,
		right
	};
	
	static const int enter_keycode;
	static const int escape_keycode;

private:
	static int viewers_count_;
	std::string window_name_;
	std::vector<std::unique_ptr<slider>> sliders_;
	cv::Mat_<cv::Vec3b> shown_image_;
	
public:
	cv::Vec3b indicator_color = cv::Vec3b(255, 100, 100);
	cv::Vec3b background_color = cv::Vec3b(0, 0, 0);
	cv::Vec3b text_color = cv::Vec3b(255, 255, 255);

	explicit viewer(int width, int height, bool resizeable = false);
	~viewer();
	
	slider& add_slider(const std::string& caption, int default_val, int max_val);
	
	void clear();
	void draw(cv::Rect rect, const cv::Mat_<cv::Vec3b>& img, real blend = 1.0);
	void draw(cv::Rect rect, const cv::Mat_<uchar>& img, real blend = 1.0);
	void draw_depth(cv::Rect rect, const cv::Mat_<float>& depth_img, float min_d, float max_d, real blend = 1.0);
	
	void draw_text(cv::Rect rect, const std::string& text, text_alignment);
	void draw_text(cv::Rect rect, const std::string& text, text_alignment, cv::Vec3b color);
	void draw_indicator(cv::Rect rect, real value, real max_value);
	void draw_2d_indicator(cv::Rect rect, real value_x, real value_y, real max_abs_value);

	bool show(int& keycode);
	bool show();
};

}

#endif
