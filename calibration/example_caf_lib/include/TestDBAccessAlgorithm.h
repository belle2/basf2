/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Tadeas Bilka                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {
  /**
   * Test class implementing calibration algorithm
   */
  class TestDBAccessAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to TestCalibration
    TestDBAccessAlgorithm();

    /// Destructor
    virtual ~TestDBAccessAlgorithm() {}

  protected:

    /// Run algo on data
    virtual EResult calibrate();

  private:
    void getAverageMean();
    void generateNewPayloads();
  };
} // namespace Belle2
