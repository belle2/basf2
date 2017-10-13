/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ALIGNABLEEKLMRECOHIT_H
#define ALIGNABLEEKLMRECOHIT_H

/* External headers. */
#include <TMatrixD.h>

/* Genfit headers. */
#include <genfit/ICalibrationParametersDerivatives.h>
#include <genfit/HMatrixUV.h>
#include <genfit/PlanarMeasurement.h>
#include <genfit/TrackCandHit.h>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMAlignmentHit.h>
#include <eklm/dataobjects/EKLMElementID.h>

namespace Belle2 {

  /**
   * Alignable EKLMHit2d.
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
    virtual std::pair<std::vector<int>, TMatrixD> globalDerivatives(const genfit::StateOnPlane* sop);

    /**
     * Clone.
     */
    genfit::AbsMeasurement* clone() const;

  private:

    /** Sector identifier. */
    EKLMElementID m_Sector;

    /** Segment identifier. */
    EKLMElementID m_Segment;

    /** V direction. */
    TVector3 m_StripV;

    /** Needed to make objects storable. */
    ClassDef(AlignableEKLMRecoHit, 2);

  };

}

#endif

