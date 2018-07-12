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
#include <framework/logging/Logger.h>
#include <TObject.h>

#include <boost/optional.hpp>

#include <vector>
#include <utility>
#include <cmath>

namespace Belle2 {
  /**
   * Storage element for the eventwise T0 estimation.
   * It can store a double value with uncertainty together with the detector component.
   *
   * Additionally, it can store an arbitrary number of temporary event t0 estimations not to be used
   * for the final decision.
   * */
  class EventT0 : public TObject {

  public:
    /// Structure for storing the extracted event t0s together with its detector and its uncertainty.
    struct EventT0Component {
      /// Empty constructor for ROOT
      EventT0Component() = default;

      /// Convenience constructor.
      EventT0Component(double eventT0_, double eventT0Uncertainty_, const Const::DetectorSet& detectorSet_,
                       const std::string& algorithm_ = "", double quality_ = NAN) :
        eventT0(eventT0_), eventT0Uncertainty(eventT0Uncertainty_), detectorSet(detectorSet_),
        algorithm(algorithm_), quality(quality_) {}

      /// Storage of the T0 estimation.
      double eventT0 = NAN;
      /// Storage of the uncertainty of the T0 estimation.
      double eventT0Uncertainty = NAN;
      /**
       * Storage of the detector, who has determined the event T0.
       * Can be multiple detectors, if the value was computed using information
       * from multiple detectors.
      */
      Const::DetectorSet detectorSet;
      /// Storage for which algorithm created the event t0
      std::string algorithm = "";
      /// Storage for the internal quality estimation for a single algorithm. Only comparable for all temporaries with the same algorithm and detector.
      double quality = NAN;

      ClassDef(EventT0Component, 3) /**< Storage element for the EventT0Component */
    };

    /// Check if a final event t0 is set
    bool hasEventT0() const;

    /// Return the final event t0, if one is set. Else, return NAN.
    double getEventT0() const;

    /// Return the final event t0, if one is set. Else, return an empty optional.
    boost::optional<EventT0Component> getEventT0Component() const;

    /// Return the final event t0 uncertainty, if one is set. Else, return NAN.
    double getEventT0Uncertainty() const;

    /// Replace/set the final double T0 estimation
    void setEventT0(double eventT0, double eventT0Uncertainty, const Const::DetectorSet& detector, const std::string& algorithm = "");

    /// Replace/set the final double T0 estimation
    void setEventT0(const EventT0Component& eventT0);

    /// Add another temporary double T0 estimation
    void addTemporaryEventT0(const EventT0Component& eventT0);

    /// Return the list of all temporary event t0 estimations.
    const std::vector<EventT0Component>& getTemporaryEventT0s() const;

    /// Return the list of all temporary event t0 estimations for a specific detector
    const std::vector<EventT0Component> getTemporaryEventT0s(Const::EDetector detector) const;

    /// Check if one of the detectors in the given set has a temporary t0 estimation.
    bool hasTemporaryEventT0(const Const::DetectorSet& detectorSet = Const::allDetectors) const;

    /// Get the detectors that have determined temporary event T0s.
    Const::DetectorSet getTemporaryDetectors() const;

    /// Return the number of stored event T0s
    unsigned long getNumberOfTemporaryEventT0s() const;

    /// Clear the list of temporary event T0 estimations
    void clearTemporaries();

    /**
     * Clear the final EventT0, this is neded in case some algorithm has
     * set one for an itertive t0 finding procedure and none was set in
     * the beginning
    */
    void clearEventT0();

  private:
    /// Internal storage of the temporary event t0 list.
    std::vector<EventT0Component> m_temporaryEventT0List;
    /// Internal storage for the final event t0
    EventT0Component m_eventT0;
    /// Internal storage of the final eventT0 is set
    bool m_hasEventT0 = false;

    ClassDef(EventT0, 5) /**< Storage element for the eventwise T0 estimation. */
  };
}
