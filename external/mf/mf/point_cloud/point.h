#ifndef MF_POINT_CLOUD_POINT_H_
#define MF_POINT_CLOUD_POINT_H_

#include "../common.h"
#include "../eigen.h"
#include "../color.h"

namespace mf {
	
static_assert(sizeof(Eigen_scalar) == 4, "point cloud requires 32 bit scalar type");


class alignas(16) point_xyz {
private:	
	Eigen_vec4 homogeneous_coordinates_ = Eigen_vec4::Zero();
	
public:
	point_xyz() = default;
	point_xyz(const point_xyz&) = default;
	point_xyz(const Eigen_vec3& pos) :
		homogeneous_coordinates_(pos[0], pos[1], pos[2], 1.0) { }
	
	point_xyz& operator=(const point_xyz&) = default;
	point_xyz& operator=(const Eigen_vec3& pos)
		{ set_position(pos); return *this; }
	
	auto position() { return homogeneous_coordinates_.head<3>(); }
	auto position() const { return homogeneous_coordinates_.head<3>(); }
	
	void set_position(const Eigen_vec3& pos)
		{ homogeneous_coordinates_ = Eigen_vec4(pos[0], pos[1], pos[2], 1.0); }
	void set_non_null() { homogeneous_coordinates_[3] = 1.0; }
	void set_null() { homogeneous_coordinates_[3] = 0.0; }
	bool is_null() const { return (homogeneous_coordinates_[3] != 1.0); }
	explicit operator bool () const { return ! is_null(); }
};
static_assert(sizeof(point_xyz) == 16, "point_xyz must be 16 byte");


class alignas(16) point_full : public point_xyz {
private:
	Eigen_vec3 normal_ = Eigen_vec3::Zero();
	rgb_color color_ = rgb_color::white;

public:
	point_full() = default;
	point_full(const point_full&) = default;
	point_full(const point_xyz& pt, const rgb_color& col = rgb_color::white, const Eigen_vec3& nor = Eigen_vec3::Zero()) :
		point_xyz(pt), normal_(nor), color_(col) { }
	point_full(const Eigen_vec3& pos, const rgb_color& col = rgb_color::white, const Eigen_vec3& nor = Eigen_vec3::Zero()) :
		point_xyz(pos), normal_(nor), color_(col) { }
		
	point_full& operator=(const point_full&) = default;
	point_full& operator=(const point_xyz& pt) { return operator=(point_full(pt)); }
	point_full& operator=(const Eigen_vec3& pos) { return operator=(point_full(pos)); }
		
	rgb_color& color() { return color_; }
	const rgb_color& color() const { return color_; }

	Eigen_vec3& normal() { return normal_; }
	const Eigen_vec3& normal() const { return normal_; }
};
static_assert(sizeof(point_full) == 32, "point_full must be 32 byte");


}

#endif
