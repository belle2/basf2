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
     * Event (hit): time, distance from hit to SiPM.
     */
    struct Event {
      float time;   /**< Time. */
      float dist;   /**< Distance. */
      float npe;    /**< Number of photoelectrons. */
    };

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

    /**
     * Turn on debug mode (prints histograms).
     */
    void setDebug();

  private:

    /** Geometry data. */
    const EKLM::GeometryData* m_GeoDat;

    /** Maximal strip number (= total number of strips). */
    int m_maxStrip;

    /** Debug mode. */
    bool m_Debug;

  };

}

#endif

