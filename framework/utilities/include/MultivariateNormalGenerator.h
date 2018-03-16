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

#include <Eigen/Dense>
#include <TRandom.h>
#include <TVectorT.h>
#include <TVector3.h>
#include <TMatrixTBase.h>

namespace Belle2 {
  /** Class to generate normal distributed, correlated random numbers given the
   * mean values and the covariance matrix of all dimensions.
   *
   * This class can be used to generate normal distributed random values
   * according to a given covariance matrix (assuming the covariance matrix is
   * positive semi-definite).
   *
   * To use it first set the desired mean values and covariance matrix using
   * setMeanCov() and then call generate() to generate one set of values.
   *
   * \warning: setMeanCov() will not work for all matrices. Please check the
   * return value when setting the covariance matrix.
   *
   * To generate normal distributed random values according to a covariance
   * matrix we need to decompose the covariance matrix \f$M\f$ into \f$M= A A^T\f$. Given
   * the vector of mean values as \f$\mu\f$ and a vector of standard normal
   * distributed random values \f$(\mu=0, \sigma=1)\f$ as n we can obtain a set of
   * correlated random values \f$x = \mu + A * n\f$.
   *
   * Usually the Cholesky decomposition is chosen as it computes \f$M = L L^T\f$.
   * However the Cholesky composition only works for positive definite matrices
   * so it does not work if for example one of the values is fixed and has no
   * error.
   *
   * To ease this restriction a little we use the LDLT decomposition given as
   * \f$M = P^T L D L^T P\f$ where \f$P\f$ is a permutation matrix and D a diagonal matrix.
   * We then can use \f$A = P^T L \sqrt(D)\f$ to caluclate the correlated values also
   * for positive semi-definite covariance matrices if the elements of D are
   * positive.
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
      //To get the correlated multivariate normal distribution, we multiply
      //standard normal distributed values (mean=0, sigma=1) with a
      //transformation matrix we obtained from an LDLT decomposition and add
      //the mean values.
      Eigen::VectorXd x(m_mean.rows());
      for (int i = 0; i < m_mean.rows(); ++i) x(i) = gRandom->Gaus();
      return m_mean + (m_transform * x);
    }
    /** reset the generator setting the size to 0. Subsequent calls to generate
     * will return 0-sized results until the generator is reinitialized using
     * setMeanCov() */
    void reset();
    /** Return the number of elements to be generated on generate() */
    size_t size() const { return m_mean.rows(); }
    /** Generate a set of correlated random numbers with the previouly set
     * mean and covariance and store them in buffer output.
     * @param output pointer to array where generated values will be stored.
     */
    void generate(double* output) const
    {
      Eigen::VectorXd x = generate();
      for (int i = 0; i < x.rows(); ++i) { output[i] = x(i); }
    }

    /** Generate a set of correlated random numbers with the previouly set
     * mean and covariance and return a TVector3. Optimally, the set mean and
     * covariance matrix should be of dimension three, otherwise just the first
     * size() elements of the TVector3 are set and the remaining elements are
     * zero. If size() is bigger than 3 the remaining values will be discarded.
     */
    TVector3 generateVec3() const
    {
      Eigen::VectorXd x = generate();
      TVector3 output(0, 0, 0);
      for (unsigned int i = 0; i < std::min(3u, (unsigned int)size()); ++i) {
        output[i] = x(i);
      }
      return output;
    }

    /** Generate a set of correlated random numbers with the previouly set
     * mean and covariance and return a TVectorT<double>
     */
    TVectorD generateVecT() const
    {
      Eigen::VectorXd x = generate();
      TVectorD output(x.rows());
      output.SetElements(x.data());
      return output;
    }

    /** set the mean and covariance for the distribution with array
     * interface: mean and covariance are passed as double arrays where the
     * covariance is expected to be an NxN matrix in row major layout
     *
     * @param n dimensionality
     * @param mean pointer to the n mean values of the distribution
     * @param cov pointer to the n*n covariance values in row major layout
     * @return true if covariance could be decomposited, false otherwise
     */
    bool setMeanCov(int n, const double* mean, const double* cov);

    /** set the mean and covariance for the distribution.
     * @param mean Vector of mean values
     * @param cov Matrix containing the covariance values
     * @return true if covariance could be decomposited, false otherwise
     */
    bool setMeanCov(const Eigen::VectorXd& mean, const Eigen::MatrixXd& cov);

    /** set mean and covariance matrix from ROOT vector/matrix objects, e.g.
     * TMatrixD, TMatrixF, TMatrixDSym and so forth
     * @param mean Vector of mean values
     * @param cov Matrix containing the covariance values
     * @return true if covariance could be decomposited, false otherwise
     */
    template<class value_type> bool setMeanCov(const TVectorT<value_type>& mean,
                                               const TMatrixTBase<value_type>& cov);

    /** set mean and covariance matrix from ROOT vector/matrix objects, e.g.
     * TMatrixD, TMatrixF, TMatrixDSym and so forth but with exactly three dimensions
     * @param mean Vector of mean values
     * @param cov Matrix containing the covariance values
     * @return true if covariance could be decomposited, false otherwise
     */
    template<class value_type> bool setMeanCov(const TVector3& mean,
                                               const TMatrixTBase<value_type>& cov);
  private:
    /** Member to store the mean values of the distribution */
    Eigen::VectorXd m_mean;
    /** Member to store the transformation matrix for standard normal
     * distributed random values */
    Eigen::MatrixXd m_transform;
  };

  template<class value_type> bool MultivariateNormalGenerator::setMeanCov(
    const TVectorT<value_type>& mean, const TMatrixTBase<value_type>& cov)
  {
    Eigen::VectorXd emean(mean.GetNrows());
    Eigen::MatrixXd ecov(cov.GetNrows(), cov.GetNcols());
    for (int i = 0; i < mean.GetNrows(); ++i) { emean(i) = mean(i); }
    for (int i = 0; i < cov.GetNrows(); ++i) {
      for (int j = 0; j < cov.GetNcols(); ++j) {
        ecov(i, j) = cov(i, j);
      }
    }
    return setMeanCov(emean, ecov);
  }

  template<class value_type> bool MultivariateNormalGenerator::setMeanCov(
    const TVector3& mean, const TMatrixTBase<value_type>& cov)
  {
    TVectorT<value_type> tmean(3);
    for (int i = 0; i < 3; ++i) {
      tmean[i] = mean[i];
    }
    return setMeanCov(tmean, cov);
  }
}
