/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/utilities/MultivariateNormalGenerator.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

void MultivariateNormalGenerator::reset()
{
  m_mean.resize(0);
  m_transform.resize(0, 0);
}

bool MultivariateNormalGenerator::setMeanCov(int n, const double* mean, const double* cov)
{
  Eigen::VectorXd emean(n);
  Eigen::MatrixXd ecov(n, n);
  //Fill the vector and matrix from the buffers in row major layout
  for (int i = 0; i < n; ++i) {
    emean[i] = mean[i];
    for (int j = 0; j < n; ++j) {
      ecov(i, j) = cov[i * n + j];
    }
  }
  //And delegate to the correct function
  return setMeanCov(emean, ecov);
}


bool MultivariateNormalGenerator::setMeanCov(const Eigen::VectorXd& mean, const Eigen::MatrixXd& cov)
{
  reset();
  if (mean.rows() != cov.rows()) {
    B2ERROR("Mean values and covariance matrix need to be of the same dimension");
    return false;
  }
  if (cov.rows() != cov.cols()) {
    B2ERROR("Covariance matrix needs to be a square matrix");
    return false;
  }

  // The usual way to calulate multivariate normal distributed values is to use
  // the cholesky decomposition C = ll' and then calculate y = mean + l * x
  // where x is a vector of standard normal distributed values. However this
  // only works for positive definite matrices so we use the LDLT composition
  // which gives us C = P'LDL'P and we compute L=P'Lsqrt(D) to get the same
  // result but in a more robust way which works also for semi-definite
  // matrices
  auto ldlt = cov.ldlt();
  if (ldlt.info() != Eigen::Success) {
    B2ERROR("Cannot compute LDLT decomposition of covariance "
            "matrix, maybe not positive semi-definite?");
    return false;
  }
  Eigen::MatrixXd L = ldlt.matrixL();
  Eigen::MatrixXd D = ldlt.vectorD().asDiagonal();
  if (D.minCoeff() < 0) {
    B2ERROR("MultivariateNormalGenerator: Negative values when computing LDL^T "
            "decomposition, cannot compute M=AA^T, resulting random numbers "
            "will not be correct");
    return false;
  }
  auto P = ldlt.transpositionsP().transpose();
  m_transform = P * L * D.cwiseSqrt();
  //And save the mean values
  m_mean = mean;
  return true;
}

