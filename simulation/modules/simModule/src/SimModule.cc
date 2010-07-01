/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/modules/simModule/SimModule.h>

#include <simulation/simkernel/B4RunManager.h>
#include <simulation/simkernel/B4DetectorConstruction.h>
#include <simulation/simkernel/B4PrimaryGeneratorAction.h>
#include <simulation/simkernel/B4EventAction.h>
#include <simulation/simkernel/B4SteppingAction.h>

#include <framework/fwcore/ModuleManager.h>
#include <framework/logging/Logger.h>
#include <boost/format.hpp>

#include "TGeoManager.h"
#include "TG4RootDetectorConstruction.h"
#include "TG4RootNavMgr.h"

#include "G4UImanager.hh"
#include "G4UIterminal.hh"
#include "G4UItcsh.hh"
#include "QGSP_BERT.hh"

#include "CLHEP/Random/Random.h"

#ifdef G4VIS_USE
#include "G4VisExecutive.hh"
#endif

using namespace std;
using namespace boost;
using namespace Belle2;

REG_MODULE(SimModule)

SimModule::SimModule() : Module("SimModule"), m_initialize(false), m_run_mgr(NULL)
#ifdef G4VIS_USE
    , m_visMgr(NULL)
#endif
{
  setDescription("SimModule");
  m_event_number = 0;

  addParam("RunId", m_runId, -9, "Set run id");
  addParam("RandomSeed", m_randomSeed, 12345, "Set random seed");
  addParam("RunVerbosity", m_runVerbose, 2, "Control run verbose level in Geant4");
  addParam("EventVerbosity", m_eventVerbose, 0, "Control event verbose level in Geant4");
  addParam("TrackingVerbosity", m_trackVerbose, 0, "Control tracking verbose level in Geant4");
  addParam("InteractiveG4", m_interactiveG4, false, "If true, interactive mode will start");
  addParam("Visualize", m_vis, false, "If true, open visualization function and visualization driver is needed, like OpenGL");
  addParam("MacroName", m_macroName, string("None"), "Macro name");

}

SimModule::~SimModule()
{
}

void SimModule::initialize()
{
  //--------------------------------------------------------------------------
  // Get TGeo from gearbox and use g4root to convert them to Geant4 geometry.
  //--------------------------------------------------------------------------
  TG4RootNavMgr* g4rootNav_mgr = TG4RootNavMgr::GetInstance(gGeoManager);

  m_run_mgr = B4RunManager::Instance();

  g4rootNav_mgr->Initialize(B4DetectorConstruction::Instance());
  g4rootNav_mgr->ConnectToG4();

  //--------------------------------------------------
  // Finish some necessary initialization in Geant4.
  //--------------------------------------------------

  // Physics
  m_run_mgr->SetUserInitialization(new QGSP_BERT);

  // Generator
  G4VUserPrimaryGeneratorAction* gen_action = new B4PrimaryGeneratorAction;
  m_run_mgr->SetUserAction(gen_action);

  // Event action
  G4UserEventAction* event_action = new B4EventAction;
  m_run_mgr->SetUserAction(event_action);

  // Step action
  G4UserSteppingAction* step_action = new B4SteppingAction;
  m_run_mgr->SetUserAction(step_action);

  //-----------------------
  // Initialize G4 kernel
  //-----------------------
  m_run_mgr->Initialize();

#ifdef G4VIS_USE
  // For visualization, but need visualization drivers, like OpenGL.
  if (m_vis) {
    INFO("Initializing G4SvcVisManager");
    m_visMgr = new G4VisExecutive;
    m_visMgr->Initialize();
  }
#endif

  //-------------------------------
  // Use user interface in Geant4.
  //-------------------------------
  G4UImanager* ui_manager = G4UImanager::GetUIpointer();

  if (!ui_manager) {
    ERROR("Can not get G4 user interface pointer.");
    exit(1);
  }

  ui_manager->ApplyCommand((format("/run/verbose %1%") % (m_runVerbose)).str());
  ui_manager->ApplyCommand((format("/event/verbose %1%") % (m_eventVerbose)).str());
  ui_manager->ApplyCommand((format("/tracking/verbose %1%") % (m_trackVerbose)).str());

  // Set macro file name
  if (m_macroName != "None") {
    INFO("Running G4 macro " << m_macroName);
    ui_manager->ApplyCommand("/control/execute " + m_macroName);
  }

  // Enable interactive mode
  if (m_interactiveG4) {
    INFO("Starting G4 terminal");

    G4UIsession * ses = new G4UIterminal(new G4UItcsh);
    ses->SessionStart();

    delete ses;
  }

  // Set run id and finish Geant4 run initialization.
  if (m_run_mgr) {
    m_run_mgr->SetRunIDCounter(m_runId);
    m_run_mgr->beginRun();
  }

  // Set random seed
  CLHEP::HepRandom::setTheSeed((long) m_randomSeed);

  return;
}

void SimModule::terminate()
{
  //--------------------
  // Delete pointers
  //--------------------
  if (m_run_mgr != NULL) {
    m_run_mgr->endRun();
    if (m_run_mgr != NULL) delete m_run_mgr;
    m_run_mgr = NULL;
  }

#ifdef G4VIS_USE
  if (m_visMgr != NULL) {
    delete m_visMgr;
  }
#endif

  return;
}

void SimModule::beginRun()
{
  /*
    if(m_run_mgr) {
      m_run_mgr->SetRunIDCounter(m_runId);
      m_run_mgr->beginRun();
    }
  */
  return;
}

void SimModule::event()
{
  //-----------------------------
  // Process a event simulation.
  //-----------------------------
  m_run_mgr->event(m_event_number);
  m_run_mgr->endOfEvent();
  m_event_number++;
}

void SimModule::endRun()
{
  //------------------
  // Delete pointers
  //------------------
  if (m_run_mgr != NULL) {
    m_run_mgr->endRun();
    if (m_run_mgr != NULL) delete m_run_mgr;
    m_run_mgr = NULL;
  }

#ifdef G4VIS_USE
  if (m_visMgr != NULL) {
    delete m_visMgr;
  }
#endif

  return;
}
