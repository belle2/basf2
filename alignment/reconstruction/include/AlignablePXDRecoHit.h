/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <pxd/reconstruction/PXDRecoHit.h>

#include <genfit/ICalibrationParametersDerivatives.h>

#include <TMatrix.h>

namespace genfit {
  class AbsMeasurement;
  class StateOnPlane;
}

namespace Belle2 {
  /** This class is used to transfer PXD information to the track fit. */
  class AlignablePXDRecoHit : public PXDRecoHit, public genfit::ICalibrationParametersDerivatives  {
    friend class PXDRecoHit;
  public:
    /// Static enabling(true) or disabling(false) addition of global derivatives for Lorentz shift
    static bool s_enableLorentzGlobalDerivatives;

    /** Inherit constructors */
    using PXDRecoHit::PXDRecoHit;

    /** Destructor. */
    virtual ~AlignablePXDRecoHit() {}

    /** Creating a deep copy of this hit. */
    genfit::AbsMeasurement* clone() const override
    {
      return new AlignablePXDRecoHit(*this);
    }

    /**
     * @brief Labels and derivatives of residuals (local measurement coordinates) w.r.t. alignment/calibration parameters
     * Matrix "G" of derivatives valid for given prediction of track state:
     *
     * G(i, j) = d_residual_i/d_parameter_j
     *
     * For 2D measurement (u,v):
     *
     * G = ( du/da du/db du/dc ... )
     *     ( dv/da dv/db dv/dc ... )
     *
     * for calibration parameters a, b, c.
     *
     * For 1D measurement:
     *
     * G = (   0     0     0   ... )
     *     ( dv/da dv/db dv/dc ... )    for V-strip,
     *
     *
     * G = ( du/da du/db du/dc ... )
     *     (   0     0     0   ... )    for U-strip,
     *
     * Measurements with more dimesions (slopes, curvature) should provide
     * full 4-5Dx(n params) matrix (state as (q/p, u', v', u, v) or (u', v', u, v))
     *
     *
     * @param sop Predicted state of the track as linearization point around
     * which derivatives of alignment/calibration parameters shall be computed
     * @return pair<vector<int>, TMatrixD> With matrix with
    * number of rows = dimension of residual, number of columns = number of parameters.
    * number of columns must match vector<int>.size().
     */
    virtual std::pair<std::vector<int>, TMatrixD> globalDerivatives(const genfit::StateOnPlane* sop) override;

  private:

    ClassDefOverride(AlignablePXDRecoHit, 3); /**< PXD RecoHit extended for alignment/calibration */
  };
}
