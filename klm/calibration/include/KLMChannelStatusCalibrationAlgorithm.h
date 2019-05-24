/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* Belle2 headers. */
#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {

  /**
   * EKLM time calibration algorithm.
   */
  class KLMChannelStatusCalibrationAlgorithm : public CalibrationAlgorithm {

  public:

    /**
     * Constructor.
     */
    KLMChannelStatusCalibrationAlgorithm();

    /**
     * Destructor.
     */
    ~KLMChannelStatusCalibrationAlgorithm();

    /**
     * Calibration.
     */
    CalibrationAlgorithm::EResult calibrate() override;

    /**
     * Set minimal average number of hits per channel required for calibration.
     */
    void setMinimalAverageHitNumber(double minimalAverageHitNumber)
    {
      m_MinimalAverageHitNumber = minimalAverageHitNumber;
    }

  protected:

    /** Minimal average number of hits per channel required for calibration. */
    double m_MinimalAverageHitNumber = 25;

  };

}
