/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <klm/dataobjects/bklm/BKLMHit2d.h>
#include <klm/bklm/geometry/Module.h>

#include <genfit/PlanarMeasurement.h>
#include <genfit/HMatrixUV.h>
#include <genfit/TrackCandHit.h>
#include <genfit/ICalibrationParametersDerivatives.h>

#include <TMatrixD.h>

namespace Belle2 {

  /**
   * Alignable BKLM hit.
   */
  class AlignableBKLMRecoHit: public genfit::PlanarMeasurement,
    public genfit::ICalibrationParametersDerivatives {

  public:

    /**
     * Constructor.
     */
    AlignableBKLMRecoHit()
    {
    }

    /**
     * Constructor.
     */
    explicit AlignableBKLMRecoHit(const BKLMHit2d* hit,
                                  const genfit::TrackCandHit* trackCandHit);

    /**
     * Destructor.
     */
    virtual ~AlignableBKLMRecoHit()
    {
    }

    /**
     * Creating a deep copy of this hit.
     */
    genfit::AbsMeasurement* clone() const override;

    /**
     * Measurement construction.
     */
    virtual std::vector<genfit::MeasurementOnPlane*>
    constructMeasurementsOnPlane(
      const genfit::StateOnPlane& state) const override;

    /**
     * Get the genfit projection H matrix (to U,V).
     */
    virtual const genfit::AbsHMatrix* constructHMatrix(
      const genfit::AbsTrackRep*) const override
    {
      return new genfit::HMatrixUV();
    };

    /** @brief Labels and derivatives of residuals (local measurement coordinates) w.r.t. alignment/calibration parameters
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

  private:

    enum { HIT_DIMENSIONS = 2 /**< sensitive Dimensions of the Hit */ };

    /** KLM module number. */
    uint16_t m_KLMModule;

    /** Hit layer. */
    int m_Layer;

    /** Module used to get geometry information. */
    const bklm::Module* m_Module; //! not streamed

    /** global coordiante of the hit. */
    CLHEP::Hep3Vector global; //! not streamed

    /** half height of scintillator module*/
    const double halfheight_sci = 0.5;

    /** Class version. */
    ClassDefOverride(AlignableBKLMRecoHit, 1);

  };

}
