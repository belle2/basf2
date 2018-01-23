/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <Eigen/Dense>

namespace EigenTypes {
  /** general dynamic matrix type */
  typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> MatrixXd;
  /** column vector type */
  typedef Eigen::Matrix<double, Eigen::Dynamic, 1> ColVector;
  //typedef Eigen::VectorXd ColVector;
  /** row vector type */
  typedef Eigen::Matrix<double, 1, Eigen::Dynamic> RowVector;
}
