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
#include <analysis/modules/TreeFitter/FitParams.h>
#include <analysis/modules/TreeFitter/ErrCode.h>

namespace TreeFitter {

  class KalmanCalculator {
  public:
    ErrCode init(const CLHEP::HepVector& value, const CLHEP::HepMatrix& G,
                 const FitParams* fitparams, const CLHEP::HepSymMatrix* V = 0, int weight = 1) ;
    void updatePar(FitParams* fitparams) ;
    void updatePar(const CLHEP::HepVector& prediction, FitParams* fitparams) ;
    void updateCov(FitParams* fitparams, double chisq = -1) ;
    double chisq() const { return m_chisq ; }
  private:
    int m_nconstraints ; // dimension of the constraint
    int m_nparameters  ; // dimension of the state
    const CLHEP::HepVector* m_value ;
    const CLHEP::HepMatrix* m_matrixG ;
    CLHEP::HepSymMatrix m_matrixR;    // cov of residual
    CLHEP::HepSymMatrix m_matrixRinv; // inverse of cov of residual
    CLHEP::HepMatrix m_matrixK   ;    // kalman gain matrix
    double m_chisq ;
    int m_ierr ;
    // some temporary results
    CLHEP::HepMatrix m_matrixCGT ;
  } ;
}

#endif //KALMANCALCULATOR_H
