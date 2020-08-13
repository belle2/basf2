/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* Belle 2 headers. */
#include <calibration/CalibrationAlgorithm.h>

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
    CalibrationAlgorithm::EResult calibrate() override;

    /**
     * Turn on debug mode (prints histograms).
     */
    void setDebug()
    {
      m_Debug = true;
    }

  private:

    /** Debug mode. */
    bool m_Debug;

  };

}
