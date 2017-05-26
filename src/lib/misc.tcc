#include <sstream>
#include <algorithm>
#include <cmath>
#include "common.h"

namespace tlz {

template<typename T, typename T2, typename T3>
inline T clamp(T value, T2 minimum, T3 maximum) {
	return std::min(std::max(value, static_cast<T>(minimum)), static_cast<T>(maximum));
}


template<typename T>
T gcd(T a, T b) {
	Assert_crit(a > 0 && b > 0);
	if(a < b) std::swap(a, b);
	while(b > 0) {
		T c = a % b;
		a = b;
		b = c;
	}
	return a;
}

template<typename T>
T lcm(T a, T b) {
	if(a == 0 || b == 0) return 0;
	else return (a * b) / gcd(a, b);
}

}
