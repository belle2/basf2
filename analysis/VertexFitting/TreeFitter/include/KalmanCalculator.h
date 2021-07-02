/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: Wouter Hulsbergen                                *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <analysis/VertexFitting/TreeFitter/EigenStackConfig.h>

#include <analysis/VertexFitting/TreeFitter/FitParams.h>
#include <analysis/VertexFitting/TreeFitter/ErrCode.h>

// inverse() needs this, in the other classes we get away with just Eigen/Core
#include <Eigen/Dense>


namespace TreeFitter {

  /** does the calculation of the gain matrix, updates the cov and fitpars */
  class KalmanCalculator {
  public:

    /** constructor  */
    KalmanCalculator(
      int sizeRes,
      int sizeState
    );

    /** update statevector */
    void updateState(FitParams& fitparams);

    /** update statevector */
    void updateState(FitParams& fitparams, FitParams& oldState);

    /** update the statevectors covariance */
    void updateCovariance(FitParams& fitparams);

    /** get chi2 of this iteration */
    double getChiSquare() { return m_chisq;}

    /** init the kalman machienery */
    ErrCode calculateGainMatrix(
      const Eigen::Matrix < double, -1, 1, 0, 7, 1 > & residuals,
      const Eigen::Matrix < double, -1, -1, 0, 7, MAX_MATRIX_SIZE > & G,
      const FitParams& fitparams,
      const Eigen::Matrix < double, -1, -1, 0, 7, 7 > * V = 0,
      double weight = 1);

    /**  get chi2 */
    double chisq() const { return m_chisq ; }

    /** get dimension of the constraint */
    double getConstraintDim() { return m_constrDim; }

  private:
    /** dimension of the constraint  */
    int m_constrDim;

    /**  chi2 */
    double m_chisq;

    /** we know the max sizes of the matrices
     *  we assume the tree is smaller than MAX_MATRIX_SIZE parameters
     *  and the largest constraint is the track constraint with 5
     *
     *  -> Eigen puts this on the stack
     *
     *  Eigen::Matrix < double, col, row, ColMajor, maxCol, maxRow>
     *  -1 = Eigen::Dynamic
     *   0 = Eigen::ColMajor (don't touch)
     * */

    /** vector holding the residuals */
    Eigen::Matrix < double, -1, 1, 0, 7, 1 > m_res;

    /** G former H, transforms covraince of {residuals}<->{x,p,E} */
    Eigen::Matrix < double, -1, -1, 0, 7, MAX_MATRIX_SIZE > m_G;

    /** R residual covariance */
    Eigen::Matrix < double, -1, -1, 0, 7, 7 > m_R;

    /** R inverse */
    Eigen::Matrix < double, -1, -1, 0, 7, 7 > m_Rinverse;

    /** K kalman gain matrix */
    Eigen::Matrix < double, -1, -1, 0, MAX_MATRIX_SIZE, 7 > m_K;

    /** C times G^t  */
    Eigen::Matrix < double, -1, -1, 0, MAX_MATRIX_SIZE, 7 > m_CGt;

  };
}
