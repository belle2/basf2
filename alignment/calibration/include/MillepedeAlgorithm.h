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
#include <alignment/PedeResult.h>
#include <alignment/PedeApplication.h>
#include <alignment/dataobjects/PedeSteering.h>

namespace Belle2 {
  /**
   * Class implementing Millepede calibration algorithm
   */
  class MillepedeAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to MillepedeCalibration
    MillepedeAlgorithm();

    /// Destructor
    virtual ~MillepedeAlgorithm() {}

    /// Get the steering to set commands etc.
    PedeSteering& steering() {return m_steering;}

    /// Get the result (invalid until executed) to get parameters etc.
    alignment::PedeResult& result() {return m_result;}

    /// Get the Pede application (for status etc.)
    alignment::PedeApplication& pede() {return m_pede;}

    /// Add (false) or subtract (true) corrections to previous values?
    void invertSign(bool use_subtraction = true) {m_invertSign = use_subtraction;}

    /// Set components (BeamParameters...) to calibrate or empty for all available in data
    void setComponents(const std::vector<std::string>& components) {m_components = components;}

    /// Report failure(false) or success (true) even if some parameters could not be determined
    void ignoreUndeterminedParams(bool ignore = true) {m_ignoreUndeterminedParams = ignore;}

    /// Set the events at which payloads can change for time-dep calibration (translation from
    /// time IDs (aka continuous subruns) to EventMetaData (and later IoVs))
    void setEvents(const std::vector<EventMetaData>& events) {m_events = events;}

  protected:

    /// Run algo on data
    virtual EResult calibrate() override;

  private:
    /// Components (BeamParameters...) to calibrate or empty for all available in data
    std::vector<std::string> m_components{};
    /// Add (true) or subtract (false) corrections?
    bool m_invertSign{false};
    /// The steering with commands
    PedeSteering m_steering{"PedeSteering.txt"};
    /// The result (invalid until execution)
    alignment::PedeResult m_result{};
    /// The Pede application (unsuccesfull until execution)
    alignment::PedeApplication m_pede{};
    /// Report failure(false) or success (true) even if some parameters could not be determined
    bool m_ignoreUndeterminedParams{false};
    /// The events at which payloads can change for time-dep calibration (translation from
    /// time IDs (aka continuous subruns) to EventMetaData (and later IoVs))
    std::vector<EventMetaData> m_events{};

    /// Write out binary files from data in tree with GBL data to be used by Millepede and add them to steering
    void prepareMilleBinary();

  };
} // namespace Belle2


