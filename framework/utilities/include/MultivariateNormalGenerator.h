/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef FRAMEWORK_UTILITIES_MULTIVARIATENORMALGENERATOR_H
#define FRAMEWORK_UTILITIES_MULTIVARIATENORMALGENERATOR_H

#include <Eigen/Dense>
#include <TRandom.h>
#include <TVectorT.h>
#include <TMatrixTBase.h>

namespace Belle2 {
  /** Class to generate normal distributed, correlated random numbers given the
   * mean values and the covariance matrix of all dimensions.
   */
  class MultivariateNormalGenerator {
  public:
    /** default constructor to allow later initialization */
    MultivariateNormalGenerator() {}
    /** constructor with array interface: mean and covariance are passed as
     * double arrays where the covariance is expected to be an NxN matrix
     * in row major layout
     * @param n dimensionality
     * @param mean pointer to the n mean values of the distribution
     * @param cov pointer to the n*n covariance values in row major layout
     */
    MultivariateNormalGenerator(int n, const double* mean, const double* cov)
    {
      setMeanCov(n, mean, cov);
    }
    /** constructor with Eigen matrix interface.
     * @param mean Vector of mean values
     * @param cov Matrix containing the covariance values
     */
    MultivariateNormalGenerator(const Eigen::VectorXd& mean, const Eigen::MatrixXd& cov)
    {
      setMeanCov(mean, cov);
    }
    /** Generate a set of correlated random numbers with the previouly set
     * mean and covariance
     * @return Vector containing the generated random numbers
     */
    Eigen::VectorXd generate() const
    {
      //To get the correlated multivariate normal distribution, we
      //multiply standard normal distributed values (mean=0, sigma=1)
      //with the cholesky decomposition and add the mean values. Since we
      //know the Cholesky to be a lower triangle matrix we use the
      //triangularView to tell Eigen to use optimized operations.
      Eigen::VectorXd x(m_mean.rows());
      for (int i = 0; i < m_mean.rows(); ++i) x(i) = gRandom->Gaus();
      return m_mean + (m_cholesky.triangularView<Eigen::Lower>() * x);
    }
    /** Generate a set of correlated random numbers with the previouly set
     * mean and covariance and store them in buffer output.
     * @param output pointer to array where generated values will be stored.
     */
    void generate(double* output) const
    {
      Eigen::VectorXd x = generate();
      for (int i = 0; i < x.rows(); ++i) { output[i] = x(i); }
    }

    /** set the mean and covariance for the distribution with array
     * interface: mean and covariance are passed as double arrays where the
     * covariance is expected to be an NxN matrix in row major layout
     * @param n dimensionality
     * @param mean pointer to the n mean values of the distribution
     * @param cov pointer to the n*n covariance values in row major layout
     */
    void setMeanCov(int n, const double* mean, const double* cov);

    /** set the mean and covariance for the distribution.
     * @param mean Vector of mean values
     * @param cov Matrix containing the covariance values
     */
    void setMeanCov(const Eigen::VectorXd& mean, const Eigen::MatrixXd& cov);

    /** set mean and covariance matrix from ROOT vector/matrix objects, e.g.
     * TMatrixD, TMatrixF, TMatrixDSym and so forth
     * @param mean Vector of mean values
     * @param cov Matrix containing the covariance values
     */
    template<class value_type> void setMeanCov(const TVectorT<value_type>& mean,
                                               const TMatrixTBase<value_type>& cov);
  private:
    /** Member to store the mean values of the distribution */
    Eigen::VectorXd m_mean;
    /** Member to store the lower triangle of the Cholesky decomposition of
     * the covariance matrix */
    Eigen::MatrixXd m_cholesky;
  };

  template<class value_type> void MultivariateNormalGenerator::setMeanCov(
    const TVectorT<value_type>& mean, const TMatrixTBase<value_type>& cov)
  {
    Eigen::VectorXd emean(mean.getNrows());
    Eigen::MatrixXd ecov(cov.getNrows(), cov.getcols());
    for (int i = 0; i < mean.getNrows(); ++i) { emean(i) = mean(i); }
    for (int i = 0; i < cov.getNrows(); ++i) {
      for (int j = 0; j < cov.getNcols(); ++j) {
        ecov(i, j) = cov(i, j);
      }
    }
    setMeanCov(emean, ecov);
  }

}

#endif // FRAMEWORK_UTILITIES_MULTIVARIATENORMALGENERATOR_H

