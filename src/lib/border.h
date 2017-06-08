#ifndef LICORNEA_BORDER_H_
#define LICORNEA_BORDER_H_

#include "json.h"

namespace tlz {

struct border {
	int top = 0;
	int left = 0;
	int right = 0;
	int bottom = 0;
		
	bool is_none() const { return (top == 0) && (left == 0) && (right == 0) && (bottom == 0); }
	explicit operator bool () const { return ! is_none(); }
};

json encode_border(const border&);
border decode_border(const json&);

cv::Size add_border(const border&, const cv::Size&);
cv::Point add_border(const border&, const cv::Point&);

}

#endif
