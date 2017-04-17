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

    /**
     * Vector of integer labels for calibration/alignment
     * parameters available (must match #columns of derivatives(...)).
     * The labels must be unique across all subdetectors in calibration.
     * @return Vector of integer labels.
     */
    std::vector<int> labels();

    /**
     * Derivatives of residuals (local measurement coordinates) w.r.t.
     * alignment/calibration parameters
     * @param[in] sop Predicted state of the track as linearization point
     *                around which derivatives of alignment/calibration
     *                parameters shall be computed
     * @return Matrix with #rows = dimension of residual, #columns = number of
     *         parameters. #columns must match labels().size().
     */
    TMatrixD derivatives(const genfit::StateOnPlane* sop);

    /**
     * Clone.
     */
    genfit::AbsMeasurement* clone() const;

  private:

    /** Segment identifier. */
    EKLMElementID m_Segment;

    /** Needed to make objects storable. */
    ClassDef(AlignableEKLMRecoHit, 1)

  };

}

#endif

