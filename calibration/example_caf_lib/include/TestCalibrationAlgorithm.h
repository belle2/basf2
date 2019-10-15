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

    /// setter for m_willFail
    void setForceFail(bool forceFail) {m_forceFail = forceFail;}

    /// getter for m_willFail
    bool getForceFail() {return m_forceFail;}

    /// setter for m_minEntries
    void setMinEntries(int minEntries) {m_minEntries = minEntries;}

    /// getter for m_minEntries
    int getMinEntries() {return m_minEntries;}

    /// setter for m_debugHisto
    void setDebugHisto(bool debugHisto) {m_debugHisto = debugHisto;}

  protected:

    /// Run algo on data
    virtual EResult calibrate() override;

  private:
    /// Set when c_NotEnoughData will be returned
    int m_minEntries = 100;
    /// Force always fail for testing
    int m_forceFail = false;
    /// Set if a debugging histogram should be created in the algorithm output directory
    int m_debugHisto = false;
    /// Perform debug histogram file creation
    void createDebugHistogram();

  };
} // namespace Belle2
