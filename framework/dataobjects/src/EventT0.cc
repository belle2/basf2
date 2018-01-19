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
  return m_eventT0.eventT0;
}

/// Return the final event t0 uncertainty, if one is set. Else, return NAN.
double EventT0::getEventT0Uncertainty() const
{
  return m_eventT0.eventT0Uncertainty;
}

/// Replace/set the final double T0 estimation
void EventT0::setEventT0(double eventT0, double eventT0Uncertainty, Const::EDetector detector)
{
  m_eventT0 = EventT0Component(eventT0, eventT0Uncertainty, detector);
  m_hasEventT0 = true;
}

bool EventT0::hasTemporaryEventT0(const Const::DetectorSet& detectorSet) const
{
  for (const EventT0Component& eventT0Component : m_temporaryEventT0List) {
    if (detectorSet.contains(eventT0Component.detector)) {
      return true;
    }
  }

  return false;
}

const std::vector<EventT0::EventT0Component>& EventT0::getTemporaryEventT0s() const
{
  return m_temporaryEventT0List;
}

Const::DetectorSet EventT0::getTemporaryDetectors() const
{
  Const::DetectorSet temporarySet;

  for (const EventT0Component& eventT0Component : m_temporaryEventT0List) {
    temporarySet += eventT0Component.detector;
  }

  return temporarySet;
}

unsigned long EventT0::getNumberOfTemporaryEventT0s() const
{
  return m_temporaryEventT0List.size();
}

void EventT0::addTemporaryEventT0(double eventT0, double eventT0Uncertainty, Const::EDetector detector)
{
  m_temporaryEventT0List.emplace_back(eventT0, eventT0Uncertainty, detector);
}

void EventT0::clearTemporaries()
{
  m_temporaryEventT0List.clear();
}