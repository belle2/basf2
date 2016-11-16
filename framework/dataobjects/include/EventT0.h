/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/gearbox/Const.h>
#include <TObject.h>

namespace Belle2 {
  /// Storage element for the eventwise T0 estimation.
  class EventT0 : public TObject {

  public:
    /// Structure for storing the extracte event t0s together with its detector and its uncertainty.
    struct EventT0Component {
      EventT0Component() {}

      EventT0Component(double eventT0, double eventT0UncertaintySquared, Const::EDetector detector) :
        eventT0(eventT0), eventT0UncertaintySquared(eventT0UncertaintySquared), detector(detector) {}

      /// Storage of the T0 estimation.
      double eventT0 = 0;

      /// Storage of the uncertainty on the eventT0 squared.
      double eventT0UncertaintySquared = 0;

      /// Storage of the detector, who has determined the event T0.
      Const::EDetector detector = Const::EDetector::invalidDetector;
    };

    /// Create a new EventT0 object to store the eventwise T0 estimation.
    EventT0() { }

    /**
     * Return the calculated eventT0 and its uncertainty using only the detectors given.
     *
     * If there is no extracted eventT0 in any of these detectors, return (0, 0).
     */
    std::pair<double, double> getEventT0WithUncertainty(const Const::DetectorSet& detectorSet = Const::allDetectors) const;

    /**
     * Return the calculated eventT0 using only the detectors given.
     *
     * If there is no extracted eventT0 in any of these detectors, return 0.
     * If you need the event t0 and the uncertainty, use better the function
     * getEventT0WithUncertainty, as it calculates both values in one step.
     */
    double getEventT0(const Const::DetectorSet& detectorSet = Const::allDetectors) const
    {
      return getEventT0WithUncertainty(detectorSet).first;
    }

    /**
     * Return the calculated eventT0 uncertainty using only the detectors given.
     *
     * If there is no extracted eventT0 in any of these detectors, return 0.
     * If you need the event t0 and the uncertainty, use better the function
     * getEventT0WithUncertainty, as it calculates both values in one step.
     */
    double getEventT0Uncertainty(const Const::DetectorSet& detectorSet = Const::allDetectors) const
    {
      return getEventT0WithUncertainty(detectorSet).second;
    }

    /// Get the detectors that have determined the event T0.
    Const::DetectorSet getDetectors() const;

    /// Return the number of stored event T0s
    unsigned long getNumberOfEventT0s() const
    {
      return m_eventT0List.size();
    }

    /// Return true if there are no stored event T0 estimations
    bool empty() const
    {
      return m_eventT0List.empty();
    }

    /// Add another T0 estimation.
    void addEventT0(double eventT0, double eventT0Uncertainty, Const::EDetector detector)
    {
      m_eventT0List.emplace_back(eventT0, eventT0Uncertainty * eventT0Uncertainty, detector);
    }

    /// Check if one of the detectors in the given set has a t0 estimation.
    bool hasEventT0(const Const::DetectorSet& detectorSet = Const::allDetectors) const;

    /// Clear the list of extracted event T0 estimations
    void clear()
    {
      m_eventT0List.clear();
    }

  private:
    /// Internal storage of the event t0 list.
    std::vector<EventT0Component> m_eventT0List;

    ClassDef(EventT0, 3) /**< Storage element for the eventwise T0 estimation. */
  };
}
