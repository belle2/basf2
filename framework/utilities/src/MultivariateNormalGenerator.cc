/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/utilities/MultivariateNormalGenerator.h>
#include <framework/logging/Logger.h>
#include <Eigen/Cholesky>

using namespace Belle2;

void MultivariateNormalGenerator::setMeanCov(int n, const double* mean, const double* cov)
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
  setMeanCov(emean, ecov);
}


void MultivariateNormalGenerator::setMeanCov(const Eigen::VectorXd& mean, const Eigen::MatrixXd& cov)
{
  m_mean.resize(0);
  m_cholesky.resize(0, 0);
  if (mean.rows() != cov.rows()) {
    B2ERROR("Mean values and covariance matrix need to be of the same dimension");
    return;
  }
  if (cov.rows() != cov.cols()) {
    B2ERROR("Covariance matrix needs to be a square matrix");
    return;
  }
  //Retrieve Cholesky decomposition and save it if possible
  Eigen::LLT<Eigen::MatrixXd> choleskyDecomposition(cov);
  if (choleskyDecomposition.info() != Eigen::Success) {
    B2ERROR("Cannot compute Cholesky decomposition of covariance "
            "matrix, maybe not positive definite?")
    return;
  }
  m_cholesky = choleskyDecomposition.matrixL();
  //And save the mean values
  m_mean = mean;
}

