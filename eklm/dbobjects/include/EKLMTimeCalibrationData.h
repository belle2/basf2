/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* External headers. */
#include <TObject.h>

namespace Belle2 {

  /**
   * EKLM time calibration data (for one strip).
   */
  class EKLMTimeCalibrationData : public TObject {

  public:

    /**
     * Constructor.
     */
    EKLMTimeCalibrationData();

    /**
     * Constructor.
     * @param[in] timeShift Time shift.
     */
    explicit EKLMTimeCalibrationData(float timeShift);

    /**
     * Destructor.
     */
    ~EKLMTimeCalibrationData();

    /**
     * Set time shift.
     */
    void setTimeShift(float timeShift);

    /**
     * Get time shift.
     */
    float getTimeShift() const;

  private:

    /** Additional rotation of the segment. */
    float m_TimeShift;

    /** Class version. */
    ClassDef(Belle2::EKLMTimeCalibrationData, 2);

  };

}
