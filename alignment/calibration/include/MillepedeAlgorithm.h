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
#include <boost/python/list.hpp>

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

  protected:

    /// Run algo on data
    virtual EResult calibrate();

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

    /// Convert IOV to string (to be able to use it as a key in map)
    std::string to_string(const IntervalOfValidity& iov)
    {
      std::stringstream stream;
      stream << iov;
      return stream.str();
    }

    /// Convert string to IOV (to convert map key back to IOV)
    IntervalOfValidity to_IOV(const std::string& iov)
    {
      std::stringstream stream;
      stream << iov;
      IntervalOfValidity IOV;
      stream >> IOV;
      return IOV;
    }

    /// Write out binary files from data in tree with GBL data to be used by Millepede and add them to steering
    void prepareMilleBinary();

  };
} // namespace Belle2


