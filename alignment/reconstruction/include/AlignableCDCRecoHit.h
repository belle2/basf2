/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <cdc/dataobjects/CDCRecoHit.h>

#include <genfit/ICalibrationParametersDerivatives.h>

namespace Belle2 {
  /// This class is used to transfer CDC information to the track fit and Millepede.
  class AlignableCDCRecoHit : public CDCRecoHit, public genfit::ICalibrationParametersDerivatives  {

  public:
    /// Static enabling(true) or disabling(false) addition of local derivative for track T0
    static bool s_enableTrackT0LocalDerivative;
    /// Static enabling(true) or disabling(false) addition of global derivative for wire sagging coefficient (per wire)
    static bool s_enableWireSaggingGlobalDerivative;
    /// Static enabling(true) or disabling(false) addition of global derivatives for wire-by-wire alignment
    static bool s_enableWireByWireAlignmentGlobalDerivatives;

    /// Inherit constructors
    using CDCRecoHit::CDCRecoHit;

    /// Destructor
    ~AlignableCDCRecoHit() {}

    /// Creating a copy of this hit.
    AlignableCDCRecoHit* clone() const override
    {
      return new AlignableCDCRecoHit(*this);
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
     * @return pair<vector<int>, TMatrixD> With matrix with #rows = dimension of residual, #columns = number of parameters.
     * #columns must match vector<int>.size().
     */
    virtual std::pair<std::vector<int>, TMatrixD> globalDerivatives(const genfit::StateOnPlane* sop) override;

    /**
     * @brief Derivatives for (local) fit parameters
     *
     * @param sop State on virtual plane to calculate derivatives
     * @return TMatrixD of local derivatives, #columns=#params, #row=2 (or measurement dimension if > 2)
     */
    virtual TMatrixD localDerivatives(const genfit::StateOnPlane* sop) override;

  private:
    /** ROOT Macro.*/
    ClassDefOverride(AlignableCDCRecoHit, 1);
  };
}
