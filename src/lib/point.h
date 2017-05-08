#ifndef LICORNEA_POINT_H_
#define LICORNEA_POINT_H_

#include "common.h"
#include "color.h"
#include "opencv.h"
#include <array>

namespace tlz {
	
static_assert(sizeof(float) == 4, "point cloud requires 32 bit float type");

struct alignas(16) point_xyz {
public:
	float x = 0.0, y = 0.0, z = 0.0, w = 0.0;
	
	point_xyz() = default;
	point_xyz(const point_xyz&) = default;
	point_xyz(float x_, float y_, float z_):
		x(x_), y(y_), z(z_), w(1.0) { }
	point_xyz(const vec3& pos) :
		point_xyz(pos[0], pos[1], pos[2]) { }
		
	point_xyz& operator=(const point_xyz&) = default;
	point_xyz& operator=(const vec3& pos)
		{ set_position(pos); return *this; }
	
	void set_position(const vec3& pos)
		{ x = pos[0]; y = pos[1]; z = pos[2]; w = 1.0; }
	vec3 position() const { return vec3(x, y, z); }
		
	void set_non_null() { w = 1.0; }
	void set_null() { w = 0.0; }
	bool is_null() const { return (w != 1.0); }
	explicit operator bool () const { return ! is_null(); }
};
static_assert(sizeof(point_xyz) == 16, "point_xyz must be 16 byte");


struct alignas(16) point_full : public point_xyz {
public:
	rgb_color color = rgb_color::white;
	int reserved : 13;

	point_full() = default;
	point_full(const point_full&) = default;
	point_full(const point_xyz& pt, const rgb_color& col = rgb_color::white) :
		point_xyz(pt), color(col) { }
		
	point_full& operator=(const point_full&) = default;
	point_full& operator=(const point_xyz& pt) { return operator=(point_full(pt)); }
	point_full& operator=(const vec3& pos) { return operator=(point_full(pos)); }
};
static_assert(sizeof(point_full) == 32, "point_full must be 32 byte");

}

#endif
