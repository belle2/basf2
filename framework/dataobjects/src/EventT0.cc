/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/dataobjects/EventT0.h>

#include <algorithm>
#include <iterator>

using namespace Belle2;

// Final event t0
/// Check if a final event t0 is set
bool EventT0::hasEventT0() const
{
  return m_hasEventT0;
}

/// Return the final event t0, if one is set. Else, return NAN.
double EventT0::getEventT0() const
{
  B2ASSERT("Not EventT0 available, but someone tried to acces it. Check with hasEventT0() method before!", hasEventT0());
  return m_eventT0.eventT0;
}

/// Return the final event t0 uncertainty, if one is set. Else, return NAN.
double EventT0::getEventT0Uncertainty() const
{
  return m_eventT0.eventT0Uncertainty;
}

/// Replace/set the final double T0 estimation
void EventT0::setEventT0(double eventT0, double eventT0Uncertainty, Const::DetectorSet detector)
{
  m_eventT0 = EventT0Component(eventT0, eventT0Uncertainty, detector);
  m_hasEventT0 = true;
}

bool EventT0::hasTemporaryEventT0(const Const::DetectorSet& detectorSet) const
{
  for (const EventT0Component& eventT0Component : m_temporaryEventT0List) {
    if (detectorSet.contains(eventT0Component.detectorSet)) {
      return true;
    }
  }

  return false;
}

const std::vector<EventT0::EventT0Component>& EventT0::getTemporaryEventT0s() const
{
  return m_temporaryEventT0List;
}

const std::vector<EventT0::EventT0Component> EventT0::getTemporaryEventT0s(Const::EDetector detector) const
{
  std::vector<EventT0::EventT0Component> detectorT0s;

  std::copy_if(m_temporaryEventT0List.begin(), m_temporaryEventT0List.end(),
  std::back_inserter(detectorT0s), [detector](EventT0::EventT0Component const & c) {return c.detectorSet.contains(detector);});
  return detectorT0s;
}


Const::DetectorSet EventT0::getTemporaryDetectors() const
{
  Const::DetectorSet temporarySet;

  for (const EventT0Component& eventT0Component : m_temporaryEventT0List) {
    temporarySet += eventT0Component.detectorSet;
  }

  return temporarySet;
}

unsigned long EventT0::getNumberOfTemporaryEventT0s() const
{
  return m_temporaryEventT0List.size();
}

void EventT0::addTemporaryEventT0(double eventT0, double eventT0Uncertainty, Const::EDetector detector)
{
  // by design, the temporary EventT0 list can only contain one detector in the
  // detector set
  m_temporaryEventT0List.emplace_back(eventT0, eventT0Uncertainty, detector);
}

void EventT0::clearTemporaries()
{
  m_temporaryEventT0List.clear();
}

void EventT0::clearEventT0()
{
  m_hasEventT0 = false;
}
