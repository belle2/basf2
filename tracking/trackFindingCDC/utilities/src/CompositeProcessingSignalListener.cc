/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/utilities/CompositeProcessingSignalListener.h>

#include <tracking/trackFindingCDC/utilities/ReversedRange.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void CompositeProcessingSignalListener::initialize()
{
  Super::initialize();
  for (ProcessingSignalListener* psl : m_subordinaryProcessingSignalListeners) {
    psl->initialize();
  }
}

void CompositeProcessingSignalListener::beginRun()
{
  Super::beginRun();
  for (ProcessingSignalListener* psl : m_subordinaryProcessingSignalListeners) {
    psl->beginRun();
  }
}

void CompositeProcessingSignalListener::beginEvent()
{
  Super::beginEvent();
  for (ProcessingSignalListener* psl : m_subordinaryProcessingSignalListeners) {
    psl->beginEvent();
  }
}

void CompositeProcessingSignalListener::endRun()
{
  for (ProcessingSignalListener* psl : reversedRange(m_subordinaryProcessingSignalListeners)) {
    psl->endRun();
  }
  Super::endRun();
}

void CompositeProcessingSignalListener::terminate()
{
  for (ProcessingSignalListener* psl : reversedRange(m_subordinaryProcessingSignalListeners)) {
    psl->terminate();
  }
  Super::terminate();
}

void CompositeProcessingSignalListener::addProcessingSignalListener(ProcessingSignalListener* psl)
{
  m_subordinaryProcessingSignalListeners.push_back(psl);
}

int CompositeProcessingSignalListener::getNProcessingSignalListener()
{
  return m_subordinaryProcessingSignalListeners.size();
}
