/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//Calculate Kalman gain matrix, update parameters and covariance matrix

#include <analysis/VertexFitting/TreeFitter/KalmanCalculator.h>

#include <iostream>
#include <framework/logging/Logger.h>
namespace TreeFitter {

  extern int vtxverbose;

  ErrCode KalmanCalculator::calculateGainMatrix(const Eigen::Matrix<double, Eigen::Dynamic, 1>& residuals,
                                                const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>& G,
                                                const FitParams* fitparams, const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>* V, double weight)
  {
    B2DEBUG(83, "------ KalmanCalculator::init                                    ");
    m_constrDim = residuals.size(); //TODO move to constrcutor?
    m_stateDim  = fitparams->getDimensionOfState();
    m_res = residuals; //TODO move to constructor?
    m_G = G;
    m_R = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>::Zero(m_constrDim, m_constrDim);
    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> C =
      Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>::Zero(m_stateDim, m_stateDim).triangularView<Eigen::Lower>();
    C  = fitparams->getCovariance().triangularView<Eigen::Lower>();
    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> Rtemp =
      Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>::Zero(m_stateDim, m_stateDim).triangularView<Eigen::Lower>();
    m_CGt = C.selfadjointView<Eigen::Lower>() * G.transpose();
    Rtemp = G * m_CGt;
    if (V && (weight) && ((*V).diagonal().array() != 0).all()) {
      const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> weightedV  = weight * (*V).selfadjointView<Eigen::Lower>();
      m_R = Rtemp + weightedV;
    } else {
      m_R = Rtemp.triangularView<Eigen::Lower>();
    }

    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> RInvtemp;
    RInvtemp = m_R.selfadjointView<Eigen::Lower>();
    m_Rinverse = RInvtemp.inverse();
    if (!m_Rinverse.allFinite()) {
      return ErrCode::inversionerror;
    }
    m_K = m_CGt * m_Rinverse.selfadjointView<Eigen::Lower>();
    m_chisq = -1;
    //std::cout << "Kalman ende covariance now: \n" <<fitparams->getCovariance()  << std::endl;
    return ErrCode::success;
  }

  void KalmanCalculator::updateState(FitParams* fitparams)
  {
    //std::cout << "residuals\n" << m_res  << std::endl;
    //std::cout << "K\n" << m_K  << std::endl;
    //std::cout << "K * residuals\n" << m_K* m_res  << std::endl;
    fitparams->getStateVector() -= m_K * m_res;
    m_chisq = m_res.transpose() * m_Rinverse.selfadjointView<Eigen::Lower>() * m_res;
  }

  void KalmanCalculator::updateCovariance(FitParams* fitparams)
  {

    //std::cout << "Photon pos cov(s) BEFORE\n" << fitparams->getCovariance().block<3, 3>(13, 13) << std::endl;

    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> fitCov  = fitparams->getCovariance().triangularView<Eigen::Lower>();
    //std::cout << "Rinv\n" << m_Rinverse  << std::endl;
    //std::cout << "m_G\n" << m_G  << std::endl;
    //std::cout << "fitCov\n" << fitCov  << std::endl;
    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> GRinvGt = m_G.transpose() * m_Rinverse.selfadjointView<Eigen::Lower>() * m_G;
    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> deltaCov  = fitCov.selfadjointView<Eigen::Lower>() * GRinvGt *
        fitCov.selfadjointView<Eigen::Lower>();
    //std::cout << "delta cov00\n" << deltaCov.block<3, 3>(13, 13) << std::endl;
    //std::cout << "delta cov44\n" << deltaCov.block<4,4>(4,4)<<std::endl;
    //JFK: somehow you cant substract triangles from each other but since its just elment wise we dont care too much 2017-09-28
    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> delta = fitCov - deltaCov;
    //std::cout << "new cov00\n" <<delta.block<4,4>(0,0) << std::endl;
    //std::cout << "new cov44\n" <<delta.block<4,4>(4,4) << std::endl;
    fitparams->getCovariance().triangularView<Eigen::Lower>() = delta.triangularView<Eigen::Lower>();
    //std::cout << "Photon momentum cov(s) AFTER\n" << fitparams->getCovariance().block<4, 4>(0,
    //          0) << "\n" << fitparams->getCovariance().block<4, 4>(4, 4)  << std::endl;
    for (int col = 0; col < m_constrDim; ++col) {
      for (int k = 0; k < m_stateDim; ++k) {
        if (m_G(col, k) != 0) {
          ++(fitparams->incrementNConstraintsVec(k));
        }
      }
    }//end for block


    //std::cout << "Photon vertex position cov AFTER\n" << fitparams->getCovariance().block<3, 3>(13, 13) << std::endl;


  }//end function

}// end namespace
