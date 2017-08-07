#ifndef LICORNEA_EIGEN_H_
#define LICORNEA_EIGEN_H_

#include "common.h"
#include <Eigen/Core>
#include <Eigen/Eigen>
#include <opencv2/core/eigen.hpp>

namespace tlz {

using Eigen_matXX = Eigen::Matrix<real, Eigen::Dynamic, Eigen::Dynamic>;
template<std::size_t Cols> using Eigen_matXn = Eigen::Matrix<real, Eigen::Dynamic, Cols>;
template<std::size_t Rows> using Eigen_matnX = Eigen::Matrix<real, Rows, Eigen::Dynamic>;
template<std::size_t Rows, std::size_t Cols> using Eigen_mat = Eigen::Matrix<real, Rows, Cols>;
using Eigen_mat33 = Eigen_mat<3, 3>;
using Eigen_mat22 = Eigen_mat<2, 2>;

using Eigen_vecX = Eigen::Matrix<real, Eigen::Dynamic, 1>;
template<std::size_t Rows> using Eigen_vec = Eigen::Matrix<real, Rows, 1>;
using Eigen_vec3 = Eigen_vec<3>;
using Eigen_vec2 = Eigen_vec<2>;

template<int Rows, int Cols>
Eigen_mat<Rows, Cols> to_eigen_mat(const cv::Matx<real, Rows, Cols>& cv_mat) {
	Eigen_mat<Rows, Cols> eigen_mat;
	for(std::ptrdiff_t i = 0; i < Rows; ++i) for(std::ptrdiff_t j = 0; j < Cols; ++j) eigen_mat(i, j) = cv_mat(i, j);
	return eigen_mat;
}
template<int Rows>
Eigen_vec<Rows> to_eigen(const cv::Vec<real, Rows>& cv_vec) {
	Eigen_vec<Rows> eigen_vec;
	for(std::ptrdiff_t i = 0; i < Rows; ++i) eigen_vec[i] = cv_vec[i];
	return eigen_vec;
}



template<std::size_t Rows, std::size_t Cols>
cv::Matx<real, Rows, Cols> from_eigen_mat(const Eigen_mat<Rows, Cols>& eigen_mat) {
	cv::Matx<real, Rows, Cols> cv_mat;
	for(std::ptrdiff_t i = 0; i < Rows; ++i) for(std::ptrdiff_t j = 0; j < Cols; ++j) cv_mat(i, j) = eigen_mat(i, j);
	return cv_mat;
}
template<int Rows>
cv::Vec<real, Rows> from_eigen(const Eigen_vec<Rows>& eigen_vec) {
	cv::Vec<real, Rows> cv_vec;
	for(std::ptrdiff_t i = 0; i < Rows; ++i) cv_vec[i] = eigen_vec[i];
	return cv_vec;
}


}

#endif
