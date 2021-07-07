/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <alignment/dataobjects/PedeSteering.h>
#include <alignment/GlobalTimeLine.h>
#include <alignment/PedeResult.h>
#include <alignment/PedeApplication.h>
#include <calibration/CalibrationAlgorithm.h>

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

    /// Set components (BeamSpot...) to calibrate or empty for all available in data
    void setComponents(const std::vector<std::string>& components) {m_components = components;}

    /// Report failure(false) or success (true) even if some parameters could not be determined
    void ignoreUndeterminedParams(bool ignore = true) {m_ignoreUndeterminedParams = ignore;}

    /// Set the events at which payloads can change for time-dep calibration (translation from
    /// time IDs (aka continuous subruns) to EventMetaData (and later IoVs))
    void setEvents(const std::vector<EventMetaData>& events) {m_events = events;}

    /// Set minimum entries - for less algo will not run, but report NotEnoughData.
    /// Use -1 for no cut (default)
    void setMinEntries(int minEntries) {m_minEntries = minEntries;}

    /// Setup the complete time dependence of parameters at once (ensures consistency) (Python version)
    ///
    /// (Calls GlobalLabel static functions internally to fill its timedep. map)
    ///
    /// @param config python list of tuples of size 2, first element is list of parameter numbers
    /// retrieved by GlobalLabel.construct(payload id (uid), element id, param id) (with empty timedep map)
    /// second is list of event metadata as tuple of size 3 (event, run, exp)
    /// For example:
    ///
    /// >>> config = [([1, 2], [(0, 0, 0), (111, 0, 0)]), ([3], (0, 3, 0))]
    /// >>> setupTimedepGlobalLabels(config)
    ///
    /// will define parameters 1 and 2 to timedep with values changing at event 0 and 111 of run 0 exp 0
    /// and (added automatically) at event 0 of run 1. Parameter 3 can change its value from run 2 to 3.
    ///
    /// Overrides the event slicing defined by setEvents()
    ///
    void setTimedepConfig(PyObject* config) {setEvents(alignment::timeline::setupTimedepGlobalLabels(config));}

    /// Setup the complete time dependence of parameters at once (ensures consistency) (C++ version)
    ///
    /// (Calls GlobalLabel static functions internally to fill its timedep. map)
    ///
    /// @param config vector of tuples of size 2, first element is vector of parameter numbers
    /// retrieved by GlobalLabel.construct(payload id (uid), element id, param id) (with empty timedep map)
    /// second is vector of event metadata as tuple of size 3 (event, run, exp)
    /// For example:
    ///
    ///     setupTimedepGlobalLabels
    ///     (
    ///         {{{1, 2}, {{0, 0, 0}, {111, 0, 0}}}, {{3}, {0, 3, 0}}}
    ///     );
    ///
    /// will define parameters 1 and 2 to be timedep with values changing at event 0 and 111 of run 0 exp 0
    /// and (added automatically) at event 0 of run 1. Parameter 3 can change its value from run 2 to 3.
    ///
    /// Overrides the event slicing defined by setEvents()
    ///
    void setTimedepConfig(std::vector< std::tuple< std::vector<int>, std::vector< std::tuple<int, int, int> > > >& config)
    {
      setEvents(alignment::timeline::setupTimedepGlobalLabels(config));
    }

  protected:

    /// Run algo on data
    virtual EResult calibrate() override;

  private:
    /// Components (BeamSpot...) to calibrate or empty for all available in data
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

    /// Minimum entries collected - report NotEnoughData for less
    int m_minEntries{ -1};

  };
} // namespace Belle2


