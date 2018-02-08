/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef KALMANCALCULATOR_H
#define KALMANCALCULATOR_H

#include <CLHEP/Matrix/Matrix.h>
#include <CLHEP/Matrix/SymMatrix.h>
#include <analysis/VertexFitting/TreeFitter/FitParams.h>
#include <analysis/VertexFitting/TreeFitter/ErrCode.h>

#include <analysis/VertexFitting/TreeFitter/EigenTypes.h>

namespace TreeFitter {

  class KalmanCalculator {
  public:
    /** constructor  */
    KalmanCalculator() : m_chisq(1e+10) {}

    /** update statevector */
    void updateState(FitParams* fitparams);

    /** update the statevectors covariance */
    void updateCovariance(FitParams* fitparams);

    /** get chi2 of this iteration */
    double getChiSquare() { return m_chisq;}

    /** init the kalman machenery */
    ErrCode calculateGainMatrix(const EigenTypes::ColVector& residuals, const EigenTypes::MatrixXd& G,
                                const FitParams* fitparams, const EigenTypes::MatrixXd* V = 0, double weight = 1);

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

    /** vecotr holding the residuals */
    EigenTypes::ColVector m_res;
    /** G former H (derivative of constraints chi2) */
    EigenTypes::MatrixXd m_G;
    /** R residual covariance */
    EigenTypes::MatrixXd m_R;
    /** R inverse */
    EigenTypes::MatrixXd m_Rinverse;
    /** K kalman gain matrix */
    EigenTypes::MatrixXd m_K;
    /** C times G^t  */
    EigenTypes::MatrixXd m_CGt;

  };
}

#endif //KALMANCALCULATOR_H
