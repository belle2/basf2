/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#pragma once

#include <vector>
#include <map>
#include <utility>
#include <Eigen/Dense>
#include <functional>
#include "TString.h"

typedef std::map<std::string, double> Pars;
typedef std::map<std::string, std::pair<double, double>> Limits;

/** properties of the fit parameter */
struct par {
  TString name; ///< name of the parameter
  double v;     ///< starting value of the parameter
  double vMin;  ///< lower limit on the parameter
  double vMax;  ///< upper limit on the parameter
};


/** Unbinned Maximum Likelihood fitter with a possibility to use Chebyshev interpolation */
struct chebFitter {

  /// Get the -2*log(p(x)) on the Cheb nodes
  Eigen::VectorXd getLogFunction(Pars pars) const;

  /// Calculate log likelihood using exact formula
  double getLogLikelihoodSlow(const Pars& pars) const;

  /// Calculate log likelihood using approximation based on Chebyshev polynomials (typically faster)
  double getLogLikelihoodFast(const Pars& pars) const;

  /// Initialize the fitter (the Chebyshev coefficients)
  void init(int Size, double xMin, double xMax);


  /// Calculate Chebyshev coefficients for the data set
  Eigen::VectorXd getDataGrid() const;

  /// Calculate Chebyshev coefficients with the covariance (useful for toy studies)
  std::pair<Eigen::VectorXd, Eigen::MatrixXd> getDataGridWithCov() const;

  /// Evaluate the fitted function approximated with the Chebyshev polynomial, typically runs faster
  double getFunctionFast(const Pars& pars, double x);

  /// Fit the data with specified initial values of parameters and limits on them
  std::pair<Pars, Eigen::MatrixXd> fitData(Pars pars, Limits limits, bool UseCheb = true);

  /// Evaluate the log likelihood
  double operator()(const double* par) const;


  std::vector<double> data;         ///< vector with the data points to be fitted
  Eigen::VectorXd dataGrid;         ///< vector with the data points related to the cheb nodes (dataGrid.size = nodes.size)
  Eigen::MatrixXd dataGridCov;      ///< vector with the data points covariances (useful for toy studies)
  Eigen::MatrixXd coefsMat;         ///< transformation matrix from chebPol to gridPoints

  Eigen::VectorXd nodes;            ///< vector with cheb nodes
  Eigen::VectorXd weights;          ///< vector with cheb weights for integration
  //int nPars;
  std::vector<std::string> parNames; ///< vector with names of the parameters
  bool useCheb;                      ///< flag to use approximation based on Chebyshev polynomials

  std::function<double(double, Pars)> myFun; ///< function to fit

};
