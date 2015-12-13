/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMTIMECALIBRATIONALGORITHM_H
#define EKLMTIMECALIBRATIONALGORITHM_H

/* Belle2 headers. */
#include <calibration/CalibrationAlgorithm.h>
#include <eklm/geometry/GeometryData.h>

namespace Belle2 {

  /**
   * EKLM time calibration algorithm.
   */
  class EKLMTimeCalibrationAlgorithm : public CalibrationAlgorithm {

  public:

    /**
     * Constructor.
     */
    EKLMTimeCalibrationAlgorithm();

    /**
     * Destructor.
     */
    ~EKLMTimeCalibrationAlgorithm();

    /**
     * Calibration.
     */
    CalibrationAlgorithm::EResult calibrate();

  private:

    /** Geometry data. */
    const EKLM::GeometryData* m_GeoDat;

    /** Number of strips with different lengths. */
    int m_nStripDifferent;

    /** To make objects storable. */
    ClassDef(EKLMTimeCalibrationAlgorithm, 1);

  };

}

#endif

