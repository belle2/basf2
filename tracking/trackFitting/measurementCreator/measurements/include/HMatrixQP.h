/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TMatrixDSymfwd.h>             // for TMatrixDSym
#include <TMatrixDfwd.h>                // for TMatrixD
#include <TVectorDfwd.h>                // for TVectorD
#include <genfit/AbsHMatrix.h>          // for AbsHMatrix

namespace Belle2 {

  /**
   * @brief AbsHMatrix implementation for one-dimensional MeasurementOnPlane and RKTrackRep parameterization.
   *
   * This projects out q/p as H = (1, 0, 0, 0, 0)
   */
  class HMatrixQP : public genfit::AbsHMatrix {

  public:

    /** Constructor. */
    HMatrixQP() {;}

    /** Return the underlying matrix. */
    const TMatrixD& getMatrix() const override;

    /** Calculate H * v = v_0. */
    TVectorD Hv(const TVectorD& v) const override;

    /** Calculate M * H^T = first column of M. */
    TMatrixD MHt(const TMatrixDSym& M) const override;

    /** Calculate M * H^T = first column of M. */
    TMatrixD MHt(const TMatrixD& M) const override;

    /** Calculate H * M * H^T = M_00. */
    void HMHt(TMatrixDSym& M) const override;

    /** Clone the matrix. */
    virtual HMatrixQP* clone() const override {return new HMatrixQP(*this);}

    /** Check for equality. */
    virtual bool isEqual(const genfit::AbsHMatrix& other) const override {return (dynamic_cast<const HMatrixQP*>(&other) != nullptr);}

    /** Print a symbol for the matrix for debugging. */
    virtual void Print(const Option_t* = "") const override;
  };

}
