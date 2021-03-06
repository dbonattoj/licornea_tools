#ifndef LICORNEA_COMMON_H_
#define LICORNEA_COMMON_H_

#include <cstddef>
#include <cstdint>
#include <string>
#include <stdexcept>
#include <utility>
#include <iostream>
#include <initializer_list>

#include <opencv2/core/core.hpp>

namespace tlz {

using real = double;
using byte = std::uint8_t;
//using uchar = std::uint8_t;
//using ushort = std::uint16_t;


using vec2 = cv::Vec<real, 2>;
using vec3 = cv::Vec<real, 3>;
using vec4 = cv::Vec<real, 4>;

using mat33 = cv::Matx<real, 3, 3>;
using mat44 = cv::Matx<real, 4, 4>;

inline vec3 mul_h(const mat44& mat, const vec3& vec) {
	vec4 in(vec[0], vec[1], vec[2], 1.0);
	vec4 out = mat * in;
	out /= out[3];
	return vec3(out[0], out[1], out[2]);
}
inline vec2 mul_h(const mat33& mat, const vec2& vec) {
	vec3 in(vec[0], vec[1], 1.0);
	vec3 out = mat * in;
	out /= out[2];
	return vec2(out[0], out[1]);
}	



template<typename In, typename Out, typename Func>
std::vector<Out> point_vec_conv_tpl(const std::vector<In>& in, Func func) {
	std::vector<Out> out(in.size());
	auto out_it = out.begin();
	auto in_it = in.cbegin();
	for(; out_it != out.end(); ++out_it, ++in_it) *out_it = func(*in_it);
	return out;
}


#define POINT_CONVERT(__name__, __in__, __out__, __expr__) \
	inline __out__ __name__(const __in__& pt) { \
		return __expr__; \
	} \
	inline std::vector<__out__> __name__(const std::vector<__in__>& pts) { \
		return point_vec_conv_tpl<__in__, __out__>(pts, static_cast<__out__(*)(const __in__&)>(&__name__)); \
	}

POINT_CONVERT(vec2_to_point2f, vec2, cv::Point2f, cv::Point2f(pt[0], pt[1]) )
POINT_CONVERT(point2f_to_vec2, cv::Point2f, vec2, vec2(pt.x, pt.y) )
POINT_CONVERT(vec2_to_point, vec2, cv::Point, cv::Point(pt[0], pt[1]) )
POINT_CONVERT(point_to_vec2, cv::Point, vec2, vec2(pt.x, pt.y) )
POINT_CONVERT(point_to_point2f, cv::Point, cv::Point2f, cv::Point2f(pt.x, pt.y) )
POINT_CONVERT(point2f_to_point, cv::Point2f, cv::Point, cv::Point(pt.x, pt.y) )


#undef POINT_CONVERT


constexpr int enter_keycode = 10;
constexpr int escape_keycode = 27;


constexpr real pi = 3.14159265358979323846;
constexpr real deg_per_rad = 180.0 / pi;
constexpr real rad_per_deg = pi / 180.0;

constexpr real golden_ratio = 1.61803398874989484820;

constexpr inline long double operator"" _deg (long double deg) {
	return deg * rad_per_deg;
}


struct index_2d {
	int x;
	int y;
	
	explicit index_2d(int x_= -1, int y_ = -1) : x(x_), y(y_) { }
	index_2d(std::initializer_list<int> lst) : x(*lst.begin()), y(*(lst.begin() + 1)) { }
};
inline bool operator==(const index_2d& a, const index_2d& b) {
	return (a.y == b.y) && (a.x == b.x);
}
inline bool operator!=(const index_2d& a, const index_2d& b) {
	return (a.y != b.y) || (a.x != b.x);
}
inline bool operator<(const index_2d& a, const index_2d& b) {
	if(a.y == b.y) return (a.x < b.x);
	else return (a.y < b.y);
}
inline bool operator<=(const index_2d& a, const index_2d& b) {
	return (a == b) || (a < b);
}
inline bool operator>(const index_2d& a, const index_2d& b) {
	return !(a <= b);
}
inline bool operator>=(const index_2d& a, const index_2d& b) {
	return (a == b) || (a > b);
}


struct view_index : index_2d {
	bool is_valid() const { return (x != -1); }
	explicit operator bool () const { return is_valid(); }
	
	bool is_1d() const { return (y == -1); }
	bool is_2d() const { return (y != -1); }

	explicit view_index(int x_ = -1, int y_ = -1) : index_2d(x_, y_) { }
};

std::string encode_view_index(view_index idx);
view_index decode_view_index(const std::string& key);

std::ostream& operator<<(std::ostream&, const view_index&);





}

#endif
