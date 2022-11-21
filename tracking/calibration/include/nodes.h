/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#pragma once

#include <Eigen/Dense>

namespace Belle2 {

  /** Get the vector of weights to calculate the integral over the Chebyshev nodes
      The nodes are by definition between 0 and 1, there are Size nodes
      To get their positions, use getNodes
   */
  Eigen::VectorXd getWeights(int Size);


  /** Get the vector of positions of the Chebyshev nodes
      The nodes are by definition between 0 and 1, there are Size nodes
      For the corresponding weights use getWeights
   */
  Eigen::VectorXd getNodes(int Size);


  /** Evaluate Chebyshev polynomials up to Size at point x
      It returns a vector of the P_i(x) for i=0..Size-1
      The polynomial is defined for x between 0 and 1
   */
  Eigen::VectorXd getPols(int Size, double x);


  /** Calculate the Chebyshev polynomials of order i=0..Size-1 at points given in vector x_j and
      sum it over point index j
      It returns  sum_j P_i(x_j) for i=0..Size-1
      The Chebyshev polynomials are defined for x between 0 and 1
   */
  Eigen::VectorXd getPolsSum(int Size, Eigen::VectorXd x);


  /** Transformation matrix between Cheb nodes and coefficients of the Cheb polynomials
      Notice, that there are two alternative ways defining polynomial interpolation:
      - coefficients c_i in of the Cheb polynomials, i.e. f(x) = sum_i c_i P_i(x)
      - Values of the f(x) in the Cheb nodes, i.e. d_j = f(x_j), where x_j are the nodes
      The Chebyshev polynomials are defined for x between 0 and 1
   */
  Eigen::MatrixXd getCoefs(int Size, bool isInverse = false);


  /** Evaluate Cheb. pol at point x when the coefficients of the expansion are provided */
  double evalPol(const Eigen::VectorXd& polCoef, double x);


  /** Transformation matrix between Cheb nodes and Cheb coefficients with better normalization of the borders */
  Eigen::MatrixXd getCoefsCheb(int oldSize);


  /** Get Interpolation vector k_i for point x from the function values at points xi (polynomial interpolation)
      In the second step, the function value at x can be evaluated as sum_i vals_i k_i
   */
  Eigen::VectorXd interpol(const Eigen::VectorXd& xi, double x);


  /** Get interpolated function value at point x when function values vals at points xi are provided.
      If the points xi are fixed and only vals are different between interpol calls, use interpol(xi, x)
      to speed up the evaluation.
   */
  double interpol(Eigen::VectorXd xi, Eigen::VectorXd vals, double x);

}
