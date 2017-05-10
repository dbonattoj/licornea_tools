#ifndef LICORNEA_OPENCV_H_
#define LICORNEA_OPENCV_H_

#include <opencv2/opencv.hpp>

#include <algorithm>
#include <stdexcept>
#include "common.h"
#include "nd.h"
#include "color.h"


namespace cv { // in OpenCV namespace
	template<>
	class DataType<::tlz::rgb_color> {
	public:
		using value_type = ::tlz::rgb_color;
		using work_type = int;
		using channel_type = uchar;
		enum {
			generic_type = 0,
			depth = DataDepth<channel_type>::value,
			channels = 3,
			fmt = ((channels - 1)<<8) + DataDepth<channel_type>::fmt,
			type = CV_MAKETYPE(depth, channels)
		};
		using vec_type = Vec<channel_type, channels>;
	};
	
	template<>
	class DataType<::tlz::ycbcr_color> {
	public:
		using value_type = ::tlz::ycbcr_color;
		using work_type = int;
		using channel_type = uchar;
		enum {
			generic_type = 0,
			depth = DataDepth<channel_type>::fmt,
			channels = 3,
			fmt = ((channels - 1)<<8) + DataDepth<channel_type>::fmt,
			type = CV_MAKETYPE(depth, channels)
		};
		using vec_type = Vec<channel_type, channels>;
	};
}


namespace tlz {


using vec4 = cv::Vec<real, 4>;
using vec3 = cv::Vec<real, 3>;
using vec2 = cv::Vec<real, 2>;
using mat33 = cv::Matx<real, 3, 3>;
using mat44 = cv::Matx<real, 4, 4>;

inline vec3 mul_h(const mat44& mat, const vec3& vec) {
	vec4 in(vec[0], vec[1], vec[2], 1.0);
	vec4 out = mat * in;
	out /= out[3];
	return vec3(out[0], out[1], out[2]);
}

/// Copy the data in \a vw into the OpenCV Mat \a mat.
/** \a mat is created and allocated as needed using `cv::Mat::create()`, and \a mat owns the new copy of the data.
 ** `Elem` can not be a `masked_elem` type. */
template<std::size_t Dim, typename Elem>
void copy_to_opencv(const ndarray_view<Dim, Elem>& vw, cv::Mat_<std::remove_const_t<Elem>>& mat) {
	int sizes[Dim];
	for(std::ptrdiff_t i = 0; i < Dim; ++i) sizes[i] = vw.shape()[i];

	mat.create(Dim, sizes);

	/// TODO check order of iterations
	std::transform(
		vw.begin(),
		vw.end(),
		mat.begin(),
		[](const Elem& elem) -> Elem { return elem; }
	);
}


/// Create `cv::Mat` header pointing at the same data as \a vw.
/** If `Elem` is a const type, a `const_cast` is done and the returned type is
 ** `const cv::Mat_<std::remove_const_t<Elem>>`. The called must ensure that the returned `cv::Mat_` does not get
 ** written into.
 ** \a vw must have default strides without padding, and `Elem` cannot be a `masked_elem` type. */
template<std::size_t Dim, typename Elem>
auto to_opencv(const ndarray_view<Dim, Elem>& vw) {	
	Assert(vw.has_default_strides_without_padding());

	using elem_type = std::remove_const_t<Elem>;
	using opencv_type = cv::DataType<elem_type>;
	//using mat_type = cv::Mat_<elem_type>;
	//using qualified_mat_type = std::conditional_t<std::is_const<Elem>::value, const mat_type, mat_type>;
	//using channel_type = typename opencv_type::channel_type;

	int sizes[Dim];
	for(std::ptrdiff_t i = 0; i < Dim; ++i) sizes[i] = vw.shape()[i];

	cv::Mat mat(
		Dim,
		sizes,
		opencv_type::type,
		reinterpret_cast<void*>(const_cast<elem_type*>(vw.start()))
	);
	cv::Mat_<elem_type> mat_(mat);

	return mat_;
}


/// Create \ref ndarray_view pointing at same data as OpenCV matrix \a mat.
/** Dimension must be specified if different from 2, because the OpenCV \a mat knows its dimension only runtime. */
template<std::size_t Dim = 2, typename Elem>
ndarray_view<Dim, Elem> to_ndarray_view(cv::Mat_<Elem>& mat) {
	if(mat.dims != Dim) throw std::invalid_argument("OpenCV matrix in to_ndarray_view has incorrect dimension");
	Elem* start = reinterpret_cast<Elem*>(mat.data);
	ndcoord<Dim, int> shape(mat.size.p, mat.size.p + Dim);
	ndcoord<Dim, std::size_t> strides(mat.step.p, mat.step.p + Dim);
	return ndarray_view<Dim, Elem>(start, shape, strides);
}


/// Create \ref ndarray_view pointing at same data as OpenCV matrix \a mat.
/** Dimension must be specified if different from 2, because the OpenCV \a mat knows its dimension only runtime. */
template<std::size_t Dim = 2, typename Elem>
ndarray_view<Dim, const Elem> to_ndarray_view(const cv::Mat_<Elem>& mat) {
	if(mat.dims != Dim) throw std::invalid_argument("OpenCV matrix in to_ndarray_view has incorrect dimension");
	const Elem* start = reinterpret_cast<const Elem*>(mat.data);
	ndcoord<Dim, int> shape(mat.size.p, mat.size.p + Dim);
	ndcoord<Dim, std::size_t> strides(mat.step.p, mat.step.p + Dim);
	return ndarray_view<Dim, const Elem>(start, shape, strides);
}



/// Copy OpenCV Mat \a mat into the memory pointed to by \a vw.
/** \a vw and the memory if points to must have been created beforehand, and \a vw must have the same shape as \a mat.
 ** `Elem` cannot be a `masked_elem` type. */
template<std::size_t Dim, typename Elem>
void copy_to_ndarray_view(cv::Mat_<Elem>& mat, const ndarray_view<Dim, Elem>& vw) {
	ndarray_view<Dim, Elem> tmp_vw = to_ndarray_view<Dim>(mat);
	if(tmp_vw.shape() != vw.shape()) throw std::invalid_argument("vw in copy_to_ndarray_view has incorrect shape");
	vw.assign(tmp_vw);
}

}

#endif
