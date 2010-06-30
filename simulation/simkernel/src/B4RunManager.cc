/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simkernel/B4RunManager.h>

using namespace Belle2;

B4RunManager* B4RunManager::m_instance = NULL;

B4RunManager* B4RunManager::Instance()
{
  if (!m_instance) m_instance = new B4RunManager();
  return m_instance;
}

B4RunManager::B4RunManager()
{
}


B4RunManager::~B4RunManager()
{
}

void B4RunManager::beginRun()
{
  // Call run initialization in G4RunManager.
  if (ConfirmBeamOnCondition()) {
    ConstructScoringWorlds();
    RunInitialization();
  }
}

void B4RunManager::event(int iEvent)
{
  //-------------------------------
  // Process one event simulation.
  //-------------------------------
  currentEvent = GenerateEvent(iEvent);
  eventManager->ProcessOneEvent(currentEvent);
  AnalyzeEvent(currentEvent);
  StackPreviousEvent(currentEvent);
}

void B4RunManager::endOfEvent()
{
}

void B4RunManager::endRun()
{
  // Call  run termination in G4RunManager.
  RunTermination();
}
