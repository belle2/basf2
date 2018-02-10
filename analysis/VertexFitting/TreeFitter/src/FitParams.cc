/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>

#include <analysis/VertexFitting/TreeFitter/FitParams.h>
#include <analysis/VertexFitting/TreeFitter/ParticleBase.h>
#include <analysis/VertexFitting/TreeFitter/EigenTypes.h>

#include <Eigen/Dense>

namespace TreeFitter {

  FitParams::FitParams(int dim)
    : m_globalState(dim),
      m_globalCovariance(dim, dim),
      m_dim(dim),
      m_chiSquare(1e10), m_nConstraints(0), m_nConstraintsVec(dim, 0)
  {
    resetStateVector();
    resetCovariance();
  }

  void FitParams::resetStateVector()
  {
    m_globalState = Eigen::Matrix<double, Eigen::Dynamic, 1>::Zero(m_dim);
  }

  void FitParams::resetCovariance()
  {
    m_globalCovariance = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>::Zero(m_dim, m_dim);

    std::fill(m_nConstraintsVec.begin(), m_nConstraintsVec.end(), 0);
    m_chiSquare = 0;
    m_nConstraints = 0;
  }

  bool FitParams::testCovariance() const
  {
    bool okay = true;
    for (int row = 0; row < m_dim && okay; ++row) {
      okay = (m_globalCovariance(row, row) > 0);
      B2DEBUG(80, "Covariance dia element is smaller than 0!");
      B2DEBUG(80, "Fitpar global cov\n" << m_globalCovariance);
    }
    return okay;
  }

  [[gnu::unused]] Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> FitParams::getMaskInCovariance(
    const std::vector<int>& indexVec) const
  {
    const int blockSize = indexVec.size();
    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> returnCov = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>::Zero(m_dim,
        m_dim);
    for (int row = 0; row < blockSize; ++row) {//JFK: in general the block is not connected otherwise Eigen would be useful here
      for (int col = 0; col < row ; ++col) {
        returnCov(row, col) = m_globalCovariance(indexVec[row], indexVec[col]);
      }
    }
    return returnCov;
  }

  [[gnu::unused]] Eigen::Matrix<double, Eigen::Dynamic, 1> FitParams::getMaskInStateVec(const std::vector<int>& indexVec) const
  {

    int nrow = indexVec.size();
    Eigen::Matrix<double, Eigen::Dynamic, 1> returnVec =  Eigen::Matrix<double, Eigen::Dynamic, 1>(nrow, 1);
    for (int row = 0; row < nrow; ++row) {
      returnVec(row) = m_globalState(indexVec[row]);
    }
    return returnVec;
  }

  [[gnu::unused]] void FitParams::resizeAndResetStateAndCov(int newdim)
  {
    if (newdim > m_dim) {
      m_dim = newdim;
      m_nConstraintsVec.resize(m_dim, 0);
      m_globalState.resize(m_dim, 1);//treated as matrix
      m_globalState = Eigen::Matrix<double, Eigen::Dynamic, 1>::Zero(m_dim);
      m_globalCovariance.resize(m_dim, m_dim);
      m_globalCovariance.triangularView<Eigen::Lower>() = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>::Zero(m_dim,
                                                          m_dim).triangularView<Eigen::Lower>();
    } else {
      B2ERROR("Cannot resize the fitparams newdim < m_dim.");
    }
  }

} //TreeFitter namespace
