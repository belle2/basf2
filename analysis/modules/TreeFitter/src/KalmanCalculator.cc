/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//Calculate Kalman gain matrix, update parameters and covariance matrix

#include <analysis/modules/TreeFitter/KalmanCalculator.h>

#include <iostream>
#include <framework/logging/Logger.h>
namespace TreeFitter {

  extern int vtxverbose;

  ErrCode KalmanCalculator::init(const EigenTypes::ColVector& residuals, const EigenTypes::MatrixXd& G,
                                 const FitParams* fitparams, const EigenTypes::MatrixXd* V, double weight)
  {
    B2DEBUG(83, "------ KalmanCalculator::init                                    ");

    m_constrDim = residuals.size();
    m_stateDim  = fitparams->getDimensionOfState();
    m_res = residuals;
    m_G = G;
    m_R = EigenTypes::MatrixXd::Zero(m_stateDim, m_stateDim);

    EigenTypes::MatrixXd C =
      EigenTypes::MatrixXd::Zero(m_stateDim, m_stateDim).triangularView<Eigen::Lower>();
    C  = fitparams->getCovariance().triangularView<Eigen::Lower>();

    EigenTypes::MatrixXd Rtemp =
      EigenTypes::MatrixXd::Zero(m_stateDim, m_stateDim).triangularView<Eigen::Lower>();
    m_CGt = C.selfadjointView<Eigen::Lower>() * G.transpose();
    Rtemp = G * m_CGt;

    if (V && (weight) && ((*V)(0, 0) != 0)) {
      const EigenTypes::MatrixXd weightedV  = weight * (*V).selfadjointView<Eigen::Lower>();
      m_R = Rtemp + weightedV;
    } else {
      m_R = Rtemp.triangularView<Eigen::Lower>();
    }

    //JFK: FIXME is this the fastest way? 2017-09-27
    EigenTypes::MatrixXd RInvtemp;
    RInvtemp = m_R.selfadjointView<Eigen::Lower>();
    m_Rinverse = RInvtemp.inverse();

    //JFK: if one of the elements is infty or nan we can stop here 2017-09-28
    // min matrix size is one
    if (!std::isfinite(m_Rinverse(0, 0))) {
      return ErrCode::inversionerror;
    }

    m_K = m_CGt * m_Rinverse.selfadjointView<Eigen::Lower>();
    m_chisq = -1;
    return ErrCode::success;
  }

  void KalmanCalculator::updateState(FitParams* fitparams)
  {
    fitparams->getStateVector() -= m_K * m_res;
    m_chisq = m_res.transpose() * m_Rinverse.selfadjointView<Eigen::Lower>() * m_res;
  }

  void KalmanCalculator::updateCovariance(FitParams* fitparams)
  {
    EigenTypes::MatrixXd fitCov;
    fitCov  = fitparams->getCovariance().triangularView<Eigen::Lower>();

    EigenTypes::MatrixXd GRinvGt = m_G.transpose() * m_Rinverse.selfadjointView<Eigen::Lower>() * m_G;
    EigenTypes::MatrixXd deltaCov;

    //JFK: since fitCov is symmetric we dont need to transpose 2017-09-27
    deltaCov  = fitCov.selfadjointView<Eigen::Lower>() * GRinvGt *
                fitCov.selfadjointView<Eigen::Lower>();

    //JFK: somehow you cant substract triangles from each other but since its just elment wise we dont care too much 2017-09-28
    EigenTypes::MatrixXd delta = fitCov - deltaCov;
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
