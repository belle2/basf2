/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <analysis/VertexFitting/TreeFitter/FitParams.h>
#include <analysis/VertexFitting/TreeFitter/ErrCode.h>
// inverse needs this in the other classes we get away with just Eigen/Core
#include <Eigen/Dense>

namespace TreeFitter {

  /** does the calculation of the gain matrix, updates the cov and fitpars */
  class KalmanCalculator {
  public:
    /** constructor  */
    KalmanCalculator() : m_chisq(1e+10) {}

    /** constructor  */
    KalmanCalculator(
      int sizeRes,
      int sizeState
    );

    /** update statevector */
    void updateState(FitParams* fitparams);

    /** update the statevectors covariance */
    void updateCovariance(FitParams* fitparams);

    /** get chi2 of this iteration */
    double getChiSquare() { return m_chisq;}

    /** init the kalman machienery */
    ErrCode calculateGainMatrix(
      const Eigen::Matrix < double, -1, 1, 0, 5, 1 > & residuals,
      const Eigen::Matrix < double, -1, -1, 0, 5, 100 > & G,
      const FitParams* fitparams,
      const Eigen::Matrix < double, -1, -1, 0, 5, 5 > * V = 0,
      double weight = 1);

    /**  get chi2 */
    double chisq() const { return m_chisq ; }

    /** get dimension of the constraint */
    double getConstraintDim() { return m_constrDim; }

  private:
    /** dimension of the constraint  */
    int m_constrDim;

    /**  dimension of the statevector */
    int m_stateDim; // dimension of the state

    /**  chi2 */
    double m_chisq;

    /** we know the max sizes of the matrices
     *  we assume the tree is smaller than 100 parameters
     *  and the largest constraint is the track constraint with 5
     *  If we are lucky Eigen puts this on the stack
     *
     *  Eigen::Matrix < double, col, row, ColMajor, maxCol, maxRow>
     *  -1 = Eigen::Dynamic
     * */

    /** vector holding the residuals */
    Eigen::Matrix < double, -1, 1, 0, 5, 1 > m_res;

    /** G former H, transforms residuals<->x,p */
    Eigen::Matrix < double, -1, -1, 0, 5, 100 > m_G;

    /** R residual covariance */
    Eigen::Matrix < double, -1, -1, 0, 5, 5 > m_R;

    /** R inverse */
    Eigen::Matrix < double, -1, -1, 0, 5, 5 > m_Rinverse;

    /** K kalman gain matrix */
    Eigen::Matrix < double, -1, -1, 0, 100, 5 > m_K;

    /** C times G^t  */
    Eigen::Matrix < double, -1, -1, 0, 100, 100 > m_CGt;

  };
}
