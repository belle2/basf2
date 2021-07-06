/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <calibration/CalibrationAlgorithm.h>
#include <algorithm>

namespace Belle2 {
  /**
   * Test class implementing calibration algorithm
   */
  class TestBoundarySettingAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to TestCalibration
    TestBoundarySettingAlgorithm();

    /// Destructor
    virtual ~TestBoundarySettingAlgorithm() {}

    /// setter for m_minEntries
    void setMinEntries(const int minEntries) {m_minEntries = minEntries;}

    /// getter for m_minEntries
    int getMinEntries() const {return m_minEntries;}

    /// Setter for boundaries
    void setBoundaries(const std::vector<Calibration::ExpRun>& boundaries) {m_requestedBoundaries = boundaries;}

    /// getter for current boundaries
    std::vector<Calibration::ExpRun> getBoundaries() const {return m_requestedBoundaries;}

  protected:

    /// Run algo on data
    virtual EResult calibrate() override;

    /// Decide if a run should be a payload boundary. Only used in certain Python Algorithm Starategies.
    virtual bool isBoundaryRequired(const Calibration::ExpRun& /*currentRun*/) override;

    /// What we do before isBoundaryRequired gets called.
    //  We reset the temporary boundaries list to be the same as the requested one.
    virtual void boundaryFindingSetup(std::vector<Calibration::ExpRun> /*runs*/, int /*iteration = 0*/) override
    {
      m_boundaries = m_requestedBoundaries;
    };

  private:

    /// Set when c_NotEnoughData will be returned
    int m_minEntries = 100;

    /// During findPayloadBoundaries and isBoundaryRequired this is used to define the boundaries we want.
    std::vector<Calibration::ExpRun> m_requestedBoundaries;

  };
} // namespace Belle2
