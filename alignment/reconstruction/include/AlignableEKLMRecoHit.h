/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/geometry/B2Vector3.h>
#include <klm/dataobjects/eklm/EKLMAlignmentHit.h>

#include <genfit/ICalibrationParametersDerivatives.h>
#include <genfit/PlanarMeasurement.h>
#include <genfit/TrackCandHit.h>

#include <TMatrixD.h>

namespace Belle2 {

  /**
   * Alignable EKLM hit.
   */
  class AlignableEKLMRecoHit: public genfit::PlanarMeasurement,
    public genfit::ICalibrationParametersDerivatives {

  public:

    /**
     * Constructor.
     */
    AlignableEKLMRecoHit();

    /**
     * Constructor.
     */
    AlignableEKLMRecoHit(const EKLMAlignmentHit* hit,
                         const genfit::TrackCandHit* trackCandHit);

    /**
     * Destructor.
     */
    ~AlignableEKLMRecoHit();

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

    /**
     * Clone.
     */
    genfit::AbsMeasurement* clone() const override;

  private:

    /** Section number. */
    int m_Section;

    /** Sector number. */
    int m_Sector;

    /** Layer number. */
    int m_Layer;

    /** KLM module number. */
    uint16_t m_KLMModule;

    /** Segment number. */
    uint16_t m_Segment;

    /** V direction. */
    B2Vector3D m_StripV;

    /** Needed to make objects storable. */
    ClassDefOverride(AlignableEKLMRecoHit, 2);

  };

}

