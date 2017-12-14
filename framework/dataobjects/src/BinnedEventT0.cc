/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/dataobjects/BinnedEventT0.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

int BinnedEventT0::getBinnedEventT0(const Const::EDetector detector) const
{
  const auto& foundEventT0 = m_eventT0Map.find(detector);
  if (foundEventT0 == m_eventT0Map.end()) {
    B2ERROR("No binned event t0 available for the given detector. Returning 0.");
    return 0;
  }

  return foundEventT0->second;
}

void BinnedEventT0::addBinnedEventT0(int eventT0, Const::EDetector detector)
{
  m_eventT0Map[detector] = eventT0;
}

bool BinnedEventT0::hasBinnedEventT0(const Const::EDetector detector) const
{
  return m_eventT0Map.find(detector) != m_eventT0Map.end();
}

void BinnedEventT0::clear()
{
  m_eventT0Map.clear();
}