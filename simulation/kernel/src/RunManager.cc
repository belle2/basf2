/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Guofu Cao                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/kernel/RunManager.h>
#include <framework/logging/Logger.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

#include <boost/foreach.hpp>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;

RunManager* RunManager::m_instance = NULL;

RunManager& RunManager::Instance()
{
  static SingletonDestroyer siDestroyer;
  if (!m_instance) m_instance = new RunManager();
  return *m_instance;
}

void RunManager::Initialize()
{
  SetGeometryToBeOptimized(true);
  //Set geometry to be Initialized because there won't be any before the beginRun
  geometryInitialized = true;
  G4RunManager::Initialize();
}


void RunManager::beginRun(int runNumber)
{
  //Initialize Geometry
  InitializeGeometry();
  //Check if all the necessary initializations have already be done.
  if (ConfirmBeamOnCondition()) {
    //Set run number
    SetRunIDCounter(runNumber);

    //Finish initialization
    ConstructScoringWorlds();
    RunInitialization();
  } else {
    B2FATAL("The Geant4 kernel was not initialized properly ! The method 'ConfirmBeamOnCondition()' failed. ")
  }
}


void RunManager::processEvent(int evtNumber)
{
  //Geant4 simulation
  currentEvent = GenerateEvent(evtNumber);
  eventManager->ProcessOneEvent(currentEvent);
  AnalyzeEvent(currentEvent);
  UpdateScoring();
  StackPreviousEvent(currentEvent);
  currentEvent = NULL;
}


void RunManager::endRun()
{
  RunTermination();
}


void RunManager::destroy()
{
  if (m_instance == NULL) return;

  delete m_instance;
  m_instance = NULL;
}


//============================================================================
//                              Private methods
//============================================================================

RunManager::RunManager() : G4RunManager()
{

}


RunManager::~RunManager()
{

}
