/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMTIMECALIBRATIONDATA_H
#define EKLMTIMECALIBRATIONDATA_H

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
    EKLMTimeCalibrationData(double timeShift);

    /**
     * Destructor.
     */
    ~EKLMTimeCalibrationData();

    /**
     * Set time shift.
     */
    void setTimeShift(double timeShift);

    /**
     * Get time shift.
     */
    double getTimeShift();

  private:

    /** Additional rotation of the segment. */
    double m_TimeShift;

    /** Makes objects storable. */
    ClassDef(Belle2::EKLMTimeCalibrationData, 1);

  };

}

#endif

