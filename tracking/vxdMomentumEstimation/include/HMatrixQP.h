/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <genfit/AbsHMatrix.h>

namespace genfit {

  /**
   * @brief AbsHMatrix implementation for one-dimensional MeasurementOnPlane and RKTrackRep parameterization.
   *
   * This projects out q/p.
   * H = (1, 0, 0, 0, 0)
   */
  class HMatrixQP : public AbsHMatrix {

  public:

    HMatrixQP() {;}

    // Return the underlying matrix
    const TMatrixD& getMatrix() const;

    // Calculate H * v = v_0
    TVectorD Hv(const TVectorD& v) const;

    // Calculate M * H^T = first column of M
    TMatrixD MHt(const TMatrixDSym& M) const;

    // Calculate M * H^T = first column of M
    TMatrixD MHt(const TMatrixD& M) const;

    // Calculate H * M * H^T = M_00
    void HMHt(TMatrixDSym& M) const;

    // Clone the matrix
    virtual HMatrixQP* clone() const {return new HMatrixQP(*this);}

    // Check for equality. Is this right????
    virtual bool isEqual(const AbsHMatrix& other) const {return (dynamic_cast<const HMatrixQP*>(&other) != NULL);}

    // Print a symbol for the matrix
    virtual void Print(const Option_t* = "") const;

    ClassDef(HMatrixQP, 1)

  };

} /* End of namespace genfit */
/** @} */
