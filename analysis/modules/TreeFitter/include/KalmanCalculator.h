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
    ErrCode init(const HepVector& value, const HepMatrix& G,
                 const FitParams* fitparams, const HepSymMatrix* V = 0, int weight = 1) ;
    void updatePar(FitParams* fitparams) ;
    void updatePar(const HepVector& prediction, FitParams* fitparams) ;
    void updateCov(FitParams* fitparams, double chisq = -1) ;
    double chisq() const { return m_chisq ; }
  private:
    int m_nconstraints ; // dimension of the constraint
    int m_nparameters  ; // dimension of the state
    const HepVector* m_value ;
    const HepMatrix* m_matrixG ;
    HepSymMatrix m_matrixR;    // cov of residual
    HepSymMatrix m_matrixRinv; // inverse of cov of residual
    HepMatrix m_matrixK   ;    // kalman gain matrix
    double m_chisq ;
    int m_ierr ;
    // some temporary results
    HepMatrix m_matrixCGT ;
  } ;
}

#endif //KALMANCALCULATOR_H
