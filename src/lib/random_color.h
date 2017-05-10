#ifndef LICORNEA_RANDOM_COLOR_H_
#define LICORNEA_RANDOM_COLOR_H_

#include "opencv.h"

namespace tlz {
	
cv::Vec3b random_color(int i, int seed = 0);

}

#endif
