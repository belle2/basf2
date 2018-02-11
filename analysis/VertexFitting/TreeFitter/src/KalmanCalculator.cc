/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
// I dont know anything about this and you did not see this.
#pragma GCC diagnostic ignored "-Wstack-usage="

#include <analysis/VertexFitting/TreeFitter/KalmanCalculator.h>

#include <iostream>
namespace TreeFitter {

  KalmanCalculator::KalmanCalculator(
    int sizeRes,
    int sizeState
  ) :
    m_constrDim(sizeRes),
    m_stateDim(sizeState),
    m_chisq(1e10),
    m_res(sizeRes),
    m_G(sizeRes, sizeState),
    m_R(sizeRes, sizeRes),
    m_Rinverse(sizeRes, sizeRes),
    m_K(sizeState, sizeRes),
    m_CGt(sizeState, sizeRes)
  {
    m_R = Eigen::Matrix < double, -1, -1, 0, 5, 5 >::Zero(m_constrDim, m_constrDim);
  }


  ErrCode KalmanCalculator::calculateGainMatrix(
    const Eigen::Matrix < double, -1, 1, 0, 5, 1 > & residuals,
    const Eigen::Matrix < double, -1, -1, 0, 5, MAX_MATRIX_SIZE > & G,
    const FitParams* fitparams,
    const Eigen::Matrix < double, -1, -1, 0, 5, 5 > * V,
    double weight)
  {
    m_res = residuals;
    m_G = G;

    Eigen::Matrix < double, -1, -1, 0, MAX_MATRIX_SIZE, MAX_MATRIX_SIZE > C =
      Eigen::Matrix < double, -1, -1, 0 , MAX_MATRIX_SIZE, MAX_MATRIX_SIZE >
      ::Zero(m_stateDim, m_stateDim).triangularView<Eigen::Lower>();


    Eigen::Matrix < double, -1, -1, 0, 5, 5 > Rtemp =
      Eigen::Matrix < double, -1, -1, 0, 5, 5 >
      ::Zero(m_constrDim, m_constrDim).triangularView<Eigen::Lower>();

    C  = fitparams->getCovariance().triangularView<Eigen::Lower>();
    m_CGt = C.selfadjointView<Eigen::Lower>() * G.transpose();

    Rtemp = G * m_CGt;
    if (V && (weight) && ((*V).diagonal().array() != 0).all()) {

      const Eigen::Matrix < double, -1, -1, 0, 5, 5 > weightedV  =
        weight * (*V).selfadjointView<Eigen::Lower>();

      m_R = Rtemp + weightedV;
    } else {
      m_R = Rtemp.triangularView<Eigen::Lower>();
    }

    Eigen::Matrix < double, -1, -1, 0, 5, 5 > RInvtemp;
    RInvtemp = m_R.selfadjointView<Eigen::Lower>();
    m_Rinverse = RInvtemp.inverse();

    if (!m_Rinverse.allFinite()) { return ErrCode::inversionerror; }

    m_K = m_CGt * m_Rinverse.selfadjointView<Eigen::Lower>();

    return ErrCode::success;
  }

  void KalmanCalculator::updateState(FitParams* fitparams)
  {
    fitparams->getStateVector() -= m_K * m_res;
    m_chisq = m_res.transpose() * m_Rinverse.selfadjointView<Eigen::Lower>() * m_res;
  }

  void KalmanCalculator::updateCovariance(FitParams* fitparams)
  {

    Eigen::Matrix < double, -1, -1, 0, MAX_MATRIX_SIZE, MAX_MATRIX_SIZE > fitCov  =
      fitparams->getCovariance().triangularView<Eigen::Lower>();

    Eigen::Matrix < double, -1, -1, 0, MAX_MATRIX_SIZE, MAX_MATRIX_SIZE > GRinvGt =
      m_G.transpose() * m_Rinverse.selfadjointView<Eigen::Lower>() * m_G;

    //fitcov is sym so no transpose needed (not that it would have runtime cost)
    Eigen::Matrix < double, -1, -1, 0, MAX_MATRIX_SIZE, MAX_MATRIX_SIZE > deltaCov  =
      fitCov.selfadjointView<Eigen::Lower>() * GRinvGt * fitCov.selfadjointView<Eigen::Lower>();

    Eigen::Matrix < double, -1, -1, 0, MAX_MATRIX_SIZE, MAX_MATRIX_SIZE > delta =
      fitCov - deltaCov;

    fitparams->getCovariance().triangularView<Eigen::Lower>() = delta.triangularView<Eigen::Lower>();

    for (int col = 0; col < m_constrDim; ++col) {
      for (int k = 0; k < m_stateDim; ++k) {
        if (m_G(col, k) != 0) {
          ++(fitparams->incrementNConstraintsVec(k));
        }
      }
    }//end for block

  }//end function

}// end namespace
