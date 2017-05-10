#include "random_color.h"
#include <random>
#include <vector>

namespace tlz {
	
cv::Vec3b random_color(int i, int seed) {
	static std::vector<cv::Vec3b> colors;
	static std::mt19937 gen(seed);
	
	if(i < colors.size()) {
		return colors[i];
	} else {
		std::uniform_int_distribution<uchar> dist(0, 255);
		colors.push_back(cv::Vec3b(dist(gen), dist(gen), dist(gen)));
		return random_color(i);
	}
}

}
