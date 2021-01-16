/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Wouter Hulsbergen,  Francesco Tenchini, Jo-Frederik Krohn *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/VertexFitting/TreeFitter/KalmanCalculator.h>

namespace TreeFitter {

  KalmanCalculator::KalmanCalculator(
    int sizeRes,
    int sizeState
  ) :
    m_constrDim(sizeRes),
    m_chisq(1e10),
    m_res(sizeRes),
    m_G(sizeRes, sizeState),
    m_R(sizeRes, sizeRes),
    m_Rinverse(sizeRes, sizeRes),
    m_K(sizeState, sizeRes),
    m_CGt(sizeState, sizeRes)
  {
    m_R = Eigen::Matrix < double, -1, -1, 0, 7, 7 >::Zero(m_constrDim, m_constrDim);
  }


  ErrCode KalmanCalculator::calculateGainMatrix(
    const Eigen::Matrix < double, -1, 1, 0, 7, 1 > & residuals,
    const Eigen::Matrix < double, -1, -1, 0, 7, MAX_MATRIX_SIZE > & G,
    const FitParams& fitparams,
    const Eigen::Matrix < double, -1, -1, 0, 7, 7 > * V,
    double weight)
  {
    m_res = residuals;
    m_G = G;

    Eigen::Matrix < double, -1, -1, 0, MAX_MATRIX_SIZE, MAX_MATRIX_SIZE > C = fitparams.getCovariance().triangularView<Eigen::Lower>();

    m_CGt = C.selfadjointView<Eigen::Lower>() * G.transpose();
    Eigen::Matrix < double, -1, -1, 0, 7, 7 > Rtemp = G * m_CGt;
    if (V && (weight) && ((*V).diagonal().array() != 0).all()) {

      const Eigen::Matrix < double, -1, -1, 0, 7, 7 > weightedV  =
        weight * (*V).selfadjointView<Eigen::Lower>();

      m_R = Rtemp + weightedV;

    } else {
      m_R = Rtemp.triangularView<Eigen::Lower>();
    }

    Eigen::Matrix < double, -1, -1, 0, 7, 7 > RInvtemp;
    RInvtemp = m_R.selfadjointView<Eigen::Lower>();
    m_Rinverse = RInvtemp.inverse();
    if (!m_Rinverse.allFinite()) { return ErrCode(ErrCode::Status::inversionerror); }

    m_K = m_CGt * m_Rinverse.selfadjointView<Eigen::Lower>();
    return ErrCode(ErrCode::Status::success);
  }

  void KalmanCalculator::updateState(FitParams& fitparams)
  {
    fitparams.getStateVector() -= m_K * m_res;
    m_chisq = m_res.transpose() * m_Rinverse.selfadjointView<Eigen::Lower>() * m_res;
  }

  void KalmanCalculator::updateState(FitParams& fitparams, FitParams& oldState)
  {
    Eigen::Matrix < double, -1, 1, 0, 7, 1 > res_prime =
      m_res + m_G * (oldState.getStateVector() - fitparams.getStateVector());
    fitparams.getStateVector() = oldState.getStateVector() -  m_K * res_prime;
    m_chisq = res_prime.transpose() * m_Rinverse.selfadjointView<Eigen::Lower>() * res_prime;
  }

  TREEFITTER_NO_STACK_WARNING

  void KalmanCalculator::updateCovariance(FitParams& fitparams)
  {
    Eigen::Matrix < double, -1, -1, 0, MAX_MATRIX_SIZE, MAX_MATRIX_SIZE > fitCov  =
      fitparams.getCovariance().triangularView<Eigen::Lower>();

    Eigen::Matrix < double, -1, -1, 0, MAX_MATRIX_SIZE, MAX_MATRIX_SIZE > GRinvGt =
      m_G.transpose() * m_Rinverse.selfadjointView<Eigen::Lower>() * m_G;

    //fitcov is sym so no transpose needed (not that it would have runtime cost)
    Eigen::Matrix < double, -1, -1, 0, MAX_MATRIX_SIZE, MAX_MATRIX_SIZE > deltaCov  =
      fitCov.selfadjointView<Eigen::Lower>() * GRinvGt * fitCov.selfadjointView<Eigen::Lower>();

    Eigen::Matrix < double, -1, -1, 0, MAX_MATRIX_SIZE, MAX_MATRIX_SIZE > delta =
      fitCov - deltaCov;

    fitparams.getCovariance().triangularView<Eigen::Lower>() = delta.triangularView<Eigen::Lower>();

  }//end function

  TREEFITTER_RESTORE_WARNINGS

}// end namespace
