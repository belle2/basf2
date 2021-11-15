/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#pragma once

#include <vector>
#include <Eigen/Dense>

namespace Belle2 {

  /** Get weights to calculate the integral over the Cheb nodes */
  Eigen::VectorXd getWeights(int Size);

  /** Get the vector with positions of the Cheb nodes */
  Eigen::VectorXd getNodes(int Size);


  /** Evaluate cheb pols up to Size at point x */
  Eigen::VectorXd getPols(int Size, double x);

  /** Evaluate sum of cheb pols up to Size at vector x, x range is between 0 and 1 */
  Eigen::VectorXd getPolsSum(int Size, Eigen::VectorXd x);


  /** Transformation matrix between Cheb nodes and Cheb coefficients */
  Eigen::MatrixXd getCoefs(int oldSize, bool isInverse = false);


  /** Transformation matrix between Cheb nodes and Cheb coefficients with better normalization of the borders */
  Eigen::MatrixXd getCoefsCheb(int oldSize);


  /** Get Interpolation vector at point x from the function values at points xi */
  Eigen::VectorXd interpol(const Eigen::VectorXd& xi, double x);


  /** Get interpolated function value at point x */
  double interpol(Eigen::VectorXd xi, Eigen::VectorXd vals, double x);

}
