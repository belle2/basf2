/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <iostream>
#include <iomanip>

#include <framework/logging/Logger.h>

#include <analysis/modules/TreeFitter/FitParams.h>
#include <analysis/modules/TreeFitter/ParticleBase.h>
#include <analysis/modules/TreeFitter/EigenTypes.h>

#include <Eigen/Dense>

namespace TreeFitter {

  FitParams::FitParams(int dim)
    : m_globalState(dim),
      m_globalCovariance(dim, dim),
      m_dim(dim),
      m_par(dim, 0),  //JFK: this is deprecated 2017-09-21
      m_cov(dim, 0), //JFK: this is deprecated 2017-09-21
      m_scale(dim, 1), //JFK: this is deprecated 2017-09-21
      m_chiSquare(1e10), m_nConstraints(0), m_nConstraintsVec(dim, 0)
  {
    resetStateVector();
    resetCovariance();
  }

  FitParams::~FitParams() {}




  void FitParams::resetPar() //JFK: this is deprecated 2017-09-21
  {
    for (int row = 1; row <= m_dim; ++row) {
      m_par(row) = 0;
    }
  }

  void FitParams::resetStateVector()
  {
    m_globalState = EigenTypes::ColVector::Zero(m_dim);
  }


  void FitParams::resetCov(double scale) //JFK: this is deprecated 2017-09-21
  {
    for (int row = 1; row <= m_dim; ++row) {
      for (int col = 1; col < row; ++col) {
        m_cov.fast(row, col) = 0;
      }
      m_cov.fast(row, row) *= scale;
      if (m_cov.fast(row, row) < 0) {
        m_cov.fast(row, row) *= -1;
      }
    }

    m_chiSquare = 0;
    m_nConstraints = 0;

    for (int row = 1; row <= m_dim; ++row) {
      nConstraintsVec(row) = 0;
    }
  }

  void FitParams::resetCovariance()
  {
    // m_globalCovariance.triangularView<Eigen::Lower>() = EigenTypes::MatrixXd::Zero(m_dim, m_dim).triangularView<Eigen::Lower>();
    //JFK: be safe ... 2017-09-30
    m_globalCovariance = EigenTypes::MatrixXd::Zero(m_dim, m_dim);

    std::fill(m_nConstraintsVec.begin(), m_nConstraintsVec.end(), 0);
    m_chiSquare = 0;
    m_nConstraints = 0;
  }

  bool FitParams::testCov() const //JFK: this is deprecated 2017-09-21
  {
    bool okay = true;
    for (int row = 1; row <= m_dim && okay; ++row) {
      okay = m_cov.fast(row, row) > 0;
      if (!okay) {
        B2ERROR("Covariance dia element: " << row << " is smaller than 0!");
      }
    }
    return okay;
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

  CLHEP::HepSymMatrix FitParams::cov(const std::vector<int>& indexVec) const //JFK: this is deprecated 2017-09-21
  {
    int nrow = indexVec.size();
    CLHEP::HepSymMatrix thecov(nrow, 0);
    for (int row = 1; row <= nrow; ++row) {
      for (int col = 1; col <= row ; ++col) {
        thecov(row, col) = m_cov(indexVec[row - 1] + 1, indexVec[col - 1] + 1);
      }
    }
    return thecov;
  }

  EigenTypes::MatrixXd FitParams::getMaskInCovariance(const std::vector<int>& indexVec) const
  {
    int blockSize = indexVec.size();
    EigenTypes::MatrixXd returnCov = EigenTypes::MatrixXd::Zero(m_dim, m_dim);
    for (int row = 0; row < blockSize; ++row) {//JFK: in general the block is not connected otherwise Eigen would be useful here
      for (int col = 0; col < row ; ++col) {
        returnCov(row, col) = m_globalCovariance(indexVec[row], indexVec[col]);
      }
    }
    return returnCov;
  }

  CLHEP::HepVector FitParams::par(const std::vector<int>& indexVec) const
  {
    int nrow = indexVec.size();
    CLHEP::HepVector thepar(nrow, 0);
    for (int row = 1; row <= nrow; ++row) {
      thepar(row) = m_par(indexVec[row - 1] + 1);
    }
    return thepar;
  }

  EigenTypes::ColVector FitParams::getMaskInStateVec(const std::vector<int>& indexVec) const
  {

    int nrow = indexVec.size();
    EigenTypes::ColVector returnVec =  EigenTypes::ColVector(nrow, 1);
    for (int row = 0; row < nrow; ++row) {
      returnVec(row) = m_globalState(indexVec[row]);
    }
    return returnVec;
  }

  void FitParams::resize(int newdim) //JFK: this is deprecated 2017-09-21
  {
    if (newdim > m_dim) {
      m_dim = newdim;
      // very expensive, but okay ...
      CLHEP::HepVector newpar(newdim, 0);
      newpar.sub(1, m_par);

      CLHEP::HepSymMatrix newcov(newdim, 0);
      newcov.sub(1, m_cov);

      m_par = newpar;
      m_cov = newcov;
      m_dim = newdim;
      m_nConstraintsVec.resize(newdim, 0);
    } else {
      B2ERROR("Cannot resize the fitparams newdim < m_dim.");
    }

  }

  void FitParams::resizeAndResetStateAndCov(int newdim)
  {
    if (newdim > m_dim) {
      m_dim = newdim;
      m_nConstraintsVec.resize(m_dim, 0);
      m_globalState.resize(m_dim, 1);//treated as matrix
      m_globalState = EigenTypes::ColVector::Zero(m_dim);
      m_globalCovariance.resize(m_dim, m_dim);
      m_globalCovariance.triangularView<Eigen::Lower>() = EigenTypes::MatrixXd::Zero(m_dim, m_dim).triangularView<Eigen::Lower>();
    } else {
      B2ERROR("Cannot resize the fitparams newdim < m_dim.");
    }
  }

}
