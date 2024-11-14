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
#include <Eigen/Dense>
#include <functional>


namespace Belle2 {

  /** values of parameters in ML fit */
  typedef std::map<std::string, double> Pars;

  /** limits of parameters in ML fit */
  typedef std::map<std::string, std::pair<double, double>> Limits;


  /** Unbinned Maximum Likelihood fitter with a possibility to use Chebyshev interpolation */
  class ChebFitter {

  public:

    /** Set the fitted data and the fitted function, should be called before init */
    void setDataAndFunction(std::function<double(double, Pars)> fun, const std::vector<double>& data)
    {
      m_myFun = fun;
      m_data = data;
    }

    /// Initialize the fitter (the Chebyshev coefficients)
    void init(int Size, double xMin, double xMax);

    /// Evaluate the log likelihood
    double operator()(const double* par) const;


    /// Fit the data with specified initial values of parameters and limits on them
    std::pair<Pars, Eigen::MatrixXd> fitData(Pars pars, Limits limits, bool UseCheb = true);


  private:

    /// Get the -2*log(p(x)) on the Cheb nodes
    Eigen::VectorXd getLogFunction(Pars pars) const;

    /// Calculate log likelihood using exact formula
    double getLogLikelihoodSlow(const Pars& pars) const;

    /// Calculate log likelihood using approximation based on Chebyshev polynomials (typically faster)
    double getLogLikelihoodFast(const Pars& pars) const;


    /// Calculate Chebyshev coefficients for the data set
    Eigen::VectorXd getDataGrid() const;

    /// Calculate Chebyshev coefficients with the covariance (useful for toy studies)
    std::pair<Eigen::VectorXd, Eigen::MatrixXd> getDataGridWithCov() const;

    /// Evaluate the fitted function approximated with the Chebyshev polynomial, typically runs faster
    double getFunctionFast(const Pars& pars, double x);




    std::vector<double> m_data;         ///< vector with the data points to be fitted
    Eigen::VectorXd m_dataGrid;         ///< vector with the data points related to the cheb nodes (m_dataGrid.size = nodes.size)
    Eigen::MatrixXd m_dataGridCov;      ///< vector with the data points covariances (useful for toy studies)
    Eigen::MatrixXd m_coefsMat;         ///< transformation matrix from chebPol to gridPoints

    Eigen::VectorXd m_nodes;            ///< vector with cheb nodes
    Eigen::VectorXd m_weights;          ///< vector with cheb weights for integration

    std::vector<std::string> m_parNames; ///< vector with names of the parameters
    bool m_useCheb = true;               ///< flag to use approximation based on Chebyshev polynomials

    std::function<double(double, Pars)> m_myFun; ///< function to fit

  };

}
