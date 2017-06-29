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

using Eigen_vecX = Eigen::Matrix<real, Eigen::Dynamic, 1>;
template<std::size_t Rows> using Eigen_vec = Eigen::Matrix<real, Rows, 1>;

}

#endif
