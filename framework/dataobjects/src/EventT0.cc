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
#include <framework/logging/Logger.h>
#include <cmath>

using namespace Belle2;

std::pair<double, double> EventT0::getEventT0WithUncertainty(const Const::DetectorSet& detectorSet) const
{
  std::pair<double, double> eventT0WithUncertainty = {0, 0};
  double preFactor = 0;

  bool found = false;

  for (const EventT0Component& component : m_eventT0List) {
    if (detectorSet.getIndex(component.detector) != -1 and component.eventT0.isDoubleStored()) {
      found = true;
      const double oneOverUncertaintySquared = 1.0f / component.eventT0.getDoubleUncertaintySquared();
      eventT0WithUncertainty.first += component.eventT0.getDoubleValue()  * oneOverUncertaintySquared;
      preFactor += oneOverUncertaintySquared;
    }
  }

  if (!found) {
    B2ERROR("No double EventT0 available for the given detector set. Returning 0, 0.");
    return std::make_pair(0, 0);
  }

  eventT0WithUncertainty.first /= preFactor;
  eventT0WithUncertainty.second = std::sqrt(1.0f / preFactor);

  return eventT0WithUncertainty;
}

int EventT0::getBinnedEventT0(const Const::DetectorSet& detectorSet) const
{
  for (const EventT0Component& component : m_eventT0List) {
    if (detectorSet.getIndex(component.detector) != -1 and not component.eventT0.isDoubleStored()) {
      return component.eventT0.getIntValue();
    }
  }

  B2ERROR("No binned EventT0 available for the given detector set. Returning 0, 0.");
  return 0;
}

Const::DetectorSet EventT0::getDetectors() const
{
  Const::DetectorSet detectorSet;

  for (const EventT0Component& component : m_eventT0List) {
    detectorSet += component.detector;
  }

  return detectorSet;
}

bool EventT0::hasEventT0(const Const::DetectorSet& detectorSet) const
{
  for (const EventT0Component& component : m_eventT0List) {
    if (detectorSet.getIndex(component.detector) != -1) {
      return true;
    }
  }

  return false;
}

bool EventT0::hasDoubleEventT0(const Const::DetectorSet& detectorSet) const
{
  for (const EventT0Component& component : m_eventT0List) {
    if (detectorSet.getIndex(component.detector) != -1 and component.eventT0.isDoubleStored()) {
      return true;
    }
  }

  return false;
}

bool EventT0::hasBinnedEventT0(const Const::DetectorSet& detectorSet) const
{
  for (const EventT0Component& component : m_eventT0List) {
    if (detectorSet.getIndex(component.detector) != -1 and not component.eventT0.isDoubleStored()) {
      return true;
    }
  }

  return false;
}
