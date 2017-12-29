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

#include <vector>
#include <utility>

namespace Belle2 {
  /**
   * Storage element for the eventwise T0 estimation.
   * It can store a double value with uncertainty or a binned integer value
   * together with the detector component.
   *
   * If you want to retrieve the calculated values, you can either retrieve
   * the double values as an average over a set of detector components (with uncertainty)
   * or the binned integer values of measurement in one detector (mostly used for L1 trigger).
   *
   * This class provides the t0 results of each sub-detector (if available) or a weighted mean
   * if multiple sub-detector t0 estimations exist.
   * */
  class EventT0 : public TObject {

  public:
    /// Structure for storing an Int or a Double with uncertainty
    class IntOrDouble {
    public:
      /// Empty constructor for ROOT
      IntOrDouble() {}

      /// Constructor using two doubles
      IntOrDouble(double value, double uncertaintySquared) :
        m_doubleValue(value), m_doubleUncertaintySquared(uncertaintySquared), m_isDouble(true) {}

      /// Constructor using an int
      explicit IntOrDouble(int value) : m_intValue(value), m_isDouble(false) {}

      /// Get the int value
      int getIntValue() const
      {
        B2ASSERT("There is no int stored", not m_isDouble);
        return m_intValue;
      }

      /// Get the double value
      double getDoubleValue() const
      {
        B2ASSERT("There is no double stored", m_isDouble);
        return m_doubleValue;
      }

      /// Get the double uncertainty squared
      double getDoubleUncertaintySquared() const
      {
        B2ASSERT("There is no double stored", m_isDouble);
        return m_doubleUncertaintySquared;
      }

      /// Test if a double was stored
      bool isDoubleStored() const
      {
        return m_isDouble;
      }

    private:
      /// Stored double
      double m_doubleValue = 0;
      /// Stored double uncertainty squared
      double m_doubleUncertaintySquared = 0;
      /// Stored int
      int m_intValue = 0;
      /// Flag, if the double should be used
      bool m_isDouble = true;
    };

    /// Structure for storing the extracte event t0s together with its detector and its uncertainty.
    struct EventT0Component {
      /// Empty constructor for ROOT
      EventT0Component() {}

      /// Convenience constructor.
      EventT0Component(const IntOrDouble& eventT0_, Const::EDetector detector_) :
        eventT0(eventT0_), detector(detector_) {}

      /// Storage of the T0 estimation.
      IntOrDouble eventT0;

      /// Storage of the detector, who has determined the event T0.
      Const::EDetector detector = Const::EDetector::invalidDetector;
    };

    /// Create a new EventT0 object to store the eventwise T0 estimation.
    EventT0() = default;

    /**
     * Return the calculated eventT0 and its uncertainty using only the detectors given
     * and only those T0 estimations, that were included as a double (not the int values).
     *
     * This means, that if you want to have your T0 estimation returned here, you have
     * to supply a double value (you can include an int also).
     *
     * If there is no extracted eventT0 in any of these detectors, return (0, 0).
     *
     * If you set detectorSet = Const::allDetectors (default) a weighted mean with all available
     * sub-detector measurements will be returned, including the combined uncertainty.
     * If you set detectorSet to one specific detector, only the values provided by this
     * detector will be returned.
     */
    std::pair<double, double> getEventT0WithUncertainty(const Const::DetectorSet& detectorSet = Const::allDetectors) const;

    /**
     * Return the calculated eventT0 using only the detectors given
     * and only those T0 estimations, that were included as a double (not the int values).
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
     * Return the binned eventT0 using only the detectors given
     * and only those T0 estimations, that were included as an int (not the double values).
     *
     * If there is no extracted eventT0 in any of these detectors, return 0.
     * As we do not have an uncertainty (and we are dealing with integers), no average is created
     * if there is more than one measurement in this detector set, but only the first one in
     * the list is returned.
     * If you need the event t0 and the uncertainty in double precision, use better the function
     * getEventT0WithUncertainty, as it calculates both values in one step.
     *
     * This function should only be used in trigger setups, where the casting to double would lead to
     * rounding erros.
     */
    int getBinnedEventT0(const Const::DetectorSet& detectorSet = Const::allDetectors) const;

    /**
     * Return the calculated eventT0 uncertainty using only the detectors given.
     * and only those T0 estimations, that were included as a double (not the int values).
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
    bool isEmpty() const
    {
      return m_eventT0List.empty();
    }

    /// Add another double T0 estimation.
    void addEventT0(double eventT0, double eventT0Uncertainty, Const::EDetector detector)
    {
      m_eventT0List.emplace_back(IntOrDouble(eventT0, eventT0Uncertainty * eventT0Uncertainty), detector);
    }

    /// Add another int T0 estimation.
    void addEventT0(int eventT0, Const::EDetector detector)
    {
      m_eventT0List.emplace_back(IntOrDouble(eventT0), detector);
    }

    /// Check if one of the detectors in the given set has a t0 estimation.
    bool hasEventT0(const Const::DetectorSet& detectorSet = Const::allDetectors) const;

    /// Check if one of the detectors in the given set has a double t0 estimation.
    bool hasDoubleEventT0(const Const::DetectorSet& detectorSet = Const::allDetectors) const;

    /// Check if one of the detectors in the given set has an integer t0 estimation.
    bool hasBinnedEventT0(const Const::DetectorSet& detectorSet = Const::allDetectors) const;

    /// Clear the list of extracted event T0 estimations
    void clear()
    {
      m_eventT0List.clear();
    }

  private:
    /// Internal storage of the event t0 list.
    std::vector<EventT0Component> m_eventT0List;

    ClassDef(EventT0, 4) /**< Storage element for the eventwise T0 estimation. */
  };
}
