/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <simulation/kernel/RunManager.h>
#include <simulation/kernel/RandomEngine.h>
#include <framework/logging/Logger.h>

#include <framework/pcore/ProcHandler.h>

#include <CLHEP/Random/Random.h>

#include <G4LogicalVolumeStore.hh>
#include <G4PhysicalVolumeStore.hh>
#include <G4SolidStore.hh>
#include <G4RegionStore.hh>
#include <G4GeometryManager.hh>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;

RunManager* RunManager::m_instance = NULL;

RunManager& RunManager::Instance()
{
  if (!m_instance) m_instance = new RunManager();
  return *m_instance;
}

void RunManager::Initialize()
{
  //Set the random number engine
  CLHEP::HepRandom::setTheEngine(new RandomEngine());

  SetGeometryToBeOptimized(true);
  //Set geometry to be Initialized because there won't be any before the beginRun
  //FIXME: reverted to run independent
  //geometryInitialized = true;
  G4RunManager::Initialize();
}


void RunManager::beginRun(int runNumber)
{
  //Initialize Geometry FIXME: done run independent at the moment
  //InitializeGeometry();
  //Check if all the necessary initializations have already be done.
  if (ConfirmBeamOnCondition()) {
    //Set run number
    SetRunIDCounter(runNumber);

    //Finish initialization
    ConstructScoringWorlds();
    RunInitialization();
  } else {
    B2FATAL("The Geant4 kernel was not initialized properly ! The method 'ConfirmBeamOnCondition()' failed. ");
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
  if (m_instance == nullptr) return;

  for (G4AssemblyVolume* assemblyVolume : m_AssemblyVolumes) delete assemblyVolume;
  m_AssemblyVolumes.clear();

  if (ProcHandler::parallelProcessingUsed() and !ProcHandler::isOutputProcess()) {
    //Attention, ugly:
    //Deleting the runmanager will cause Geant4 to open the geometry and clean up
    //optimizations. This will modify the geometry and hurt parallel processing
    //memory usage so we do not want to do this, the operating system
    //will free the memory anyway.
    //So what we do if we are in one of the child processes is to empty the
    //list of pointers in the G4LogicalVolumeStore. This means Geant4 will
    //not find the volumes and will not delete the optimizations. We also clear
    //all the other stores which contain geometry information just to be sure
    G4LogicalVolumeStore::GetInstance()->clear();
    G4PhysicalVolumeStore::GetInstance()->clear();
    G4SolidStore::GetInstance()->clear();
    G4RegionStore::GetInstance()->clear();
    //And we have to open the geometry to avoid warnings from the Stores which
    //is now a trivial operation
    G4GeometryManager::GetInstance()->OpenGeometry();
  } else {
    //If we are the framework process (a.k.a the parent or only process) we do
    //the full cleanup. We could live with the same approach as for child
    //processes but then valgrind will probably show huge memory leaks
    delete m_instance;
  }
  m_instance = nullptr;
}


//============================================================================
//                              Private methods
//============================================================================

RunManager::RunManager() : G4RunManager()
{
  m_AssemblyVolumes.clear();
}


RunManager::~RunManager()
{
}
