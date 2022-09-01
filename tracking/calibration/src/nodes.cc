/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/



#include <cassert>
#include <vector>
#include <iostream>
#include <cmath>

#include <Eigen/Dense>

namespace Belle2 {

  using Eigen::VectorXd;
  using Eigen::MatrixXd;


  // to integrate function between 0 and 1 (more in nodes.h)
  VectorXd getWeights(int Size)
  {
    const int N = Size - 1;
    assert(N % 2 == 0);

    std::vector<std::vector<double>> coef(Size);
    for (auto& el : coef) el.resize(Size);


    for (int k = 0; k <= N / 2; ++k) {
      coef[2 * k][N] = 1. / N;
      coef[2 * k][0] = 1. / N ;

      coef[2 * k][N / 2] = 2. / N * (2 * ((k + 1) % 2) - 1);

      for (int n = 1; n <= N / 2 - 1; ++n)
        coef[2 * k][n] = coef[2 * k][N - n] = 2. / N * cos(n * k * M_PI * 2 / N);
    }

    VectorXd wgt = VectorXd::Zero(Size);


    for (int i = 0; i < Size; ++i) {
      wgt[i] += coef[0][i];
      wgt[i] += coef[N][i] / (1. - N * N);
      for (int k = 1; k <= N / 2 - 1; ++k) {
        double w = 2. / (1 - 4 * k * k);
        wgt[i] += w * coef[2 * k][i];
      }

      wgt[i] *= 0.5; //for interval (0,1)
    }
    return wgt;
  }

  // vector of nodes to integrate function between 0 and 1 (more in nodes.h)
  VectorXd getNodes(int Size)
  {
    assert((Size - 1) % 2 == 0);
    VectorXd xi = VectorXd::Zero(Size);
    for (int i = 0; i < Size; ++i) {
      double Cos = cos(i / (Size - 1.) * M_PI);
      xi[i] = (1 - Cos) / 2;
    }
    return xi;
  }

// Evaluate Cheb pols up to Size at point x, x is between 0 and 1 (more in nodes.h)
  VectorXd getPols(int Size, double x)
  {
    VectorXd pol(Size);
    double C = 2 * (2 * x - 1);

    if (Size >= 1) pol[0] = 1;
    if (Size >= 2) pol[1] = C / 2;

    for (int i = 2; i < Size; ++i)
      pol[i] = C * pol[i - 1] - pol[i - 2];
    return pol;
  }


  /** Calculate the Chebyshev polynomials of order i=0..Size-1 at points given in vector x_j and
      sum it over point index j
      It returns  sum_j P_i(x_j) for i=0..Size-1
      The Chebyshev polynomials are defined for x between 0 and 1
   */
  VectorXd getPolsSum(int Size, VectorXd x)
  {
    assert(Size > 2);

    VectorXd polSum(Size);

    VectorXd pol0 = 0 * x.array() + 1;
    VectorXd pol1 = 2 * x.array() - 1;
    VectorXd C    = 2 * pol1;

    VectorXd pol2(x.size());
    for (int i = 2; i < Size; ++i) {
      polSum(i - 2) = pol0.sum();

      pol2 = C.array() * pol1.array() - pol0.array();

      pol0 = pol1;
      pol1 = pol2;
    }

    polSum(Size - 2) = pol0.sum();
    polSum(Size - 1) = pol1.sum();

    return polSum;
  }





// Transformation matrix between Cheb nodes and Cheb. Coefficients (more in nodes.h)
  MatrixXd getCoefs(int Size, bool isInverse = false)
  {
    const int N = Size - 1;
    assert(N % 2 == 0);

    MatrixXd  coef(Size, Size);

    double mul = 1;
    double C = 1. / N;
    if (isInverse == true) {C = 1. / 2; }

    for (int k = 0; k <= N; ++k) {
      if (!isInverse) {
        coef(k, N) = C;
        coef(k, 0) = C * (k % 2 == 1 ? -1 : 1);
      } else {
        mul = k % 2 == 1 ? -1 : 1;
        coef(N - k, N) = C * mul;
        coef(N - k, 0) = C ;
      }

      for (int n = 1; n <= N - 1; ++n) {
        double el = cos(n * k * M_PI / N) * 2.*C * mul;
        if (!isInverse) coef(k, N - n) = el;
        else           coef(N - k, N - n) = el;
      }
    }

    return coef;
  }




// getCoef with better normalization of the borders (more in nodes.h)
  MatrixXd getCoefsCheb(int oldSize)
  {
    auto coef = getCoefs(oldSize);

    coef.row(0) *= 0.5;
    coef.row(coef.rows() - 1) *= 0.5;

    return coef;
  }




  /** Evaluate Cheb. pol at point x when the coefficients of the expansion are provided */
  double evalPol(const VectorXd& polCoef, double x)
  {
    VectorXd pols = getPols(polCoef.size(), x);

    double s = pols.dot(polCoef);

    return s;
  }


  /** Get Interpolation vector k_i for point x from the function values at points xi (polynomial interpolation)
      In the second step, the function value at x can be evaluated as sum_i vals_i k_i
   */
  VectorXd interpol(const VectorXd& xi, double x)
  {
    double Norm = (xi[xi.size() - 1] - xi[0]) / 2;
    VectorXd coefs(xi.size());
    for (int i = 0; i < xi.size(); ++i) {
      double num = 1, den = 1;
      for (int j = 0; j < xi.size(); ++j)
        if (j != i) {
          num *= (x     - xi(j)) / Norm;
          den *= (xi(i) - xi(j)) / Norm;
        }
      coefs(i) = num / den;
    }
    return coefs;
  }


  /** Get interpolated function value at point x when function values vals at points xi are provided.
      If the points xi are fixed and only vals are different between interpol calls, use interpol(xi, x)
      to speed up the evaluation.
   */
  double interpol(VectorXd xi, VectorXd vals, double x)
  {
    VectorXd coefs = interpol(xi, x);
    return coefs.dot(vals);
  }

}
