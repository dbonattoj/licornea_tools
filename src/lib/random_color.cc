#include "random_color.h"
#include "misc.h"
#include <random>
#include <vector>

namespace tlz {
	
cv::Vec3b random_color(int i) {
	std::mt19937 gen(i + 1);
	std::uniform_int_distribution<int> dist(0, 255);
	return cv::Vec3b(dist(gen), dist(gen), dist(gen));
}

}
