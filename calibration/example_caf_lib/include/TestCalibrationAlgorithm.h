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
  class TestCalibrationAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to TestCalibration
    TestCalibrationAlgorithm();

    /// Destructor
    virtual ~TestCalibrationAlgorithm() {}

    /// Setter for m_minEntries
    void setMinEntries(int minEntries) {m_minEntries = minEntries;}

    /// Getter for m_minEntries
    int getMinEntries() {return m_minEntries;}

  protected:

    /// Run algo on data
    virtual EResult calibrate();

  private:
    int m_minEntries = 100;

  };
} // namespace Belle2
