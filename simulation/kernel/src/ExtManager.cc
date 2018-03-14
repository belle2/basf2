/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Leo Piilonen                                            *
 * Derived from:  G4ErrorPropagatorManager                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/kernel/ExtManager.h>
#include <simulation/kernel/RunManager.h>
#include <simulation/kernel/MagneticField.h>
#include <simulation/kernel/DetectorConstruction.h>
#include <simulation/kernel/ExtPhysicsList.h>

#include <vector>
#include <string>

#include <G4UImanager.hh>
#include <G4ErrorPropagator.hh>
#include <G4ErrorRunManagerHelper.hh>
#include <G4RunManager.hh>
#include <G4EventManager.hh>
#include <G4RegionStore.hh>
#include <G4ProductionCutsTable.hh>
#include <G4ErrorPropagatorData.hh>
#include <G4ErrorPropagator.hh>
#include <G4ErrorTrajErr.hh>
#include <G4ErrorFreeTrajState.hh>
#include <G4StateManager.hh>
#include <G4TransportationManager.hh>
#include <G4FieldManager.hh>
#include <G4CachedMagneticField.hh>
#include <G4Mag_UsualEqRhs.hh>
#include <G4ErrorMag_UsualEqRhs.hh>
#include <G4MagIntegratorStepper.hh>
#include <G4NystromRK4.hh>
#include <G4HelixExplicitEuler.hh>
#include <G4HelixSimpleRunge.hh>
#include <G4EquationOfMotion.hh>
#include <G4ChordFinder.hh>
#include <G4HadronicProcessStore.hh>
#include <G4LossTableManager.hh>
#include <G4VisExecutive.hh>
#include <G4VExceptionHandler.hh>

#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace Belle2::Simulation;

ExtManager* ExtManager::m_Manager = NULL;

//! Class to handle G4Exception raised during PropagateOneStep()
class StepExceptionHandler: public G4VExceptionHandler {
public:
  //! Constructor
  StepExceptionHandler() {}
  //! Destructor
  ~StepExceptionHandler() {}
  //! G4VExceptionHandler method called when an exception is raised
  virtual bool Notify(const char* origin, const char* code, G4ExceptionSeverity, const char* description)
  {
    // Is this an exception for low-momentum track that would over-curl in B field?
    if (strstr(description, "Error returned: 3") != NULL) {
      B2DEBUG(1, "In " << origin << ", " << code << ": " << description);
    } else {
      B2ERROR("In " << origin << ", " << code << ": " << description);
    }
    return false;
  }
};

ExtManager* ExtManager::GetManager()
{
  if (m_Manager == NULL) m_Manager = new ExtManager;
  return m_Manager;
}

ExtManager::ExtManager() :
  m_Propagator(NULL),
  m_G4RunMgr(NULL),
  m_TrackingAction(NULL),
  m_SteppingAction(NULL),
  m_MagneticField(NULL),
  m_UncachedField(NULL),
  m_MagFldEquation(NULL),
  m_Stepper(NULL),
  m_ChordFinder(NULL),
  m_StdStepper(NULL),
  m_ForwardEquationOfMotion(NULL),
  m_BackwardEquationOfMotion(NULL),
  m_VisManager(NULL)
{
  // This flag will be PreInit if FullSimModule is not present
  m_G4State = G4StateManager::GetStateManager()->GetCurrentState();
  G4ErrorPropagatorData::GetErrorPropagatorData()->SetState(G4ErrorState_PreInit);
  // Create a private handler for G4Exceptions during PropagateOneStep
  m_StepExceptionHandler = new StepExceptionHandler();
}

ExtManager::~ExtManager()
{
  delete m_StepExceptionHandler;
  if (m_Propagator) { delete m_Propagator; m_Propagator = NULL; }
}

void ExtManager::InitTrackPropagation(G4ErrorMode mode)
{
  if (m_G4RunMgr) {
    m_G4RunMgr->SetUserAction((G4UserTrackingAction*)NULL);
    m_G4RunMgr->SetUserAction((G4UserSteppingAction*)NULL);
  }
  if (mode == G4ErrorMode_PropBackwards) {
    if (m_StdStepper) {
      m_StdStepper->SetEquationOfMotion(m_BackwardEquationOfMotion);
    }
  }
  if (m_Propagator == NULL) m_Propagator = new G4ErrorPropagator();
  m_Propagator->SetStepN(0);
  G4ErrorPropagatorData::GetErrorPropagatorData()->SetState(G4ErrorState_Propagating);
}

G4int ExtManager::PropagateOneStep(G4ErrorTrajState* currentTS, G4ErrorMode mode)
{
  G4ErrorPropagatorData::GetErrorPropagatorData()->SetMode(mode);
  // Remember the existing G4Exception handler
  G4VExceptionHandler* savedHandler = G4StateManager::GetStateManager()->GetExceptionHandler();
  // Set our own exception handler temporarily
  G4StateManager::GetStateManager()->SetExceptionHandler(m_StepExceptionHandler);
  // Propagate one step via geant4e
  G4int result = m_Propagator->PropagateOneStep(currentTS);
  // Restore the saved exception handler
  G4StateManager::GetStateManager()->SetExceptionHandler(savedHandler);
  return result;
}

void ExtManager::EventTermination(G4ErrorMode mode)
{
  G4ErrorPropagatorData::GetErrorPropagatorData()->SetState(G4ErrorState_Init);
  if (m_G4RunMgr) {
    m_G4RunMgr->SetUserAction(m_TrackingAction);
    m_G4RunMgr->SetUserAction(m_SteppingAction);
  }
  if (mode == G4ErrorMode_PropBackwards) {
    if (m_StdStepper) {
      m_StdStepper->SetEquationOfMotion(m_ForwardEquationOfMotion);
    }
  }
}

void ExtManager::RunTermination()
{
  if (G4ErrorPropagatorData::GetErrorPropagatorData()->GetState() == G4ErrorState_PreInit) return;

  if (m_G4RunMgr) {
    m_G4RunMgr->SetUserAction(m_TrackingAction);
    m_G4RunMgr->SetUserAction(m_SteppingAction);
  } else {
    // Copied from FullSimModule
    //We used one Geant4 run for all Belle2 runs so end the geant4 run here
    Simulation::RunManager& myRunMgr = Simulation::RunManager::Instance();
    myRunMgr.endRun();
    //And clean up the run manager
    if (m_VisManager) delete m_VisManager;
    myRunMgr.destroy();
    // Delete the objects associated with transport in magnetic field
    if (m_ChordFinder) delete m_ChordFinder;
    if (m_Stepper) delete m_Stepper;
    if (m_MagFldEquation) delete m_MagFldEquation;
    if (m_UncachedField) delete m_UncachedField;
    if (m_MagneticField) delete m_MagneticField;
  }
  if (m_BackwardEquationOfMotion) delete m_BackwardEquationOfMotion;
  G4ErrorPropagatorData::GetErrorPropagatorData()->SetState(G4ErrorState_PreInit);
}

void ExtManager::Initialize(const char caller[], const std::string& magneticFieldName,
                            double magneticCacheDistance,
                            double deltaChordInMagneticField,
                            bool enableVisualization,
                            int trackingVerbosity,
                            const std::vector<std::string>& uiCommands)
{

  int status = (m_G4State == G4State_PreInit) ? 0 : 2;

  if (G4ErrorPropagatorData::GetErrorPropagatorData()->GetState() == G4ErrorState_Init) {
    status += 1;
    if (status == 1) {
      B2INFO("ExtManager::Initialize(): " << caller << " will run without FullSim; the extrapolator has already been initialized");
    } else {
      B2INFO("ExtManager::Initialize(): " << caller << " will run with FullSim; the extrapolator has already been initialized");
    }
    return;
  }

  G4FieldManager* fieldManager = G4TransportationManager::GetTransportationManager()->GetFieldManager();
  if (status == 2) {
    m_G4RunMgr = G4RunManager::GetRunManager();
    m_TrackingAction = const_cast<G4UserTrackingAction*>(m_G4RunMgr->GetUserTrackingAction());
    m_SteppingAction = const_cast<G4UserSteppingAction*>(m_G4RunMgr->GetUserSteppingAction());
  } else { // status == 0

    Simulation::RunManager& myRunMgr = Simulation::RunManager::Instance();

    // This is duplicated from FullSimModule::initialize() to use the Geant4/Geant4e
    // machinery for extrapolation only (no simulation)

    // Create the magnetic field for the geant4e extrapolation
    if (magneticFieldName != "none") {
      m_MagneticField = new Simulation::MagneticField();
      if (magneticCacheDistance > 0.0) {
        m_UncachedField = m_MagneticField;
        m_MagneticField = new G4CachedMagneticField(m_UncachedField, magneticCacheDistance);
      }
      fieldManager->SetDetectorField(m_MagneticField);
      if (magneticFieldName != "default") {

        //We only use Magnetic field so let's try the specialized steppers
        m_MagFldEquation = new G4Mag_UsualEqRhs(m_MagneticField);
        if (magneticFieldName == "nystrom") {
          m_Stepper = new G4NystromRK4(m_MagFldEquation);
        } else if (magneticFieldName == "expliciteuler") {
          m_Stepper = new G4HelixExplicitEuler(m_MagFldEquation);
        } else if (magneticFieldName == "simplerunge") {
          m_Stepper = new G4HelixSimpleRunge(m_MagFldEquation);
        } else {
          B2FATAL("Unknown magnetic field option: " << magneticFieldName);
        }

        //Set a minimum stepsize (stepMinimum): The chordfinder should not attempt to limit
        //the stepsize to something less than 10µm (which is the default value of Geant4).
        m_ChordFinder = new G4ChordFinder(m_MagneticField, 1e-2 * CLHEP::mm, m_Stepper);
        fieldManager->SetChordFinder(m_ChordFinder);
      } else {
        fieldManager->CreateChordFinder(m_MagneticField);
      }

      //Change DeltaCord (the max. miss-distance between the trajectory curve and its linear chord(s) approximation, if asked.
      G4ChordFinder* chordFinder = fieldManager->GetChordFinder();
      B2DEBUG(1, "Geant4 default deltaChord = " << chordFinder->GetDeltaChord());
      chordFinder->SetDeltaChord(deltaChordInMagneticField * CLHEP::mm);
      B2DEBUG(1, "DeltaChord after reset = " << chordFinder->GetDeltaChord());
      //This might be a good place to optimize the Integration parameters (DeltaOneStep, DeltaIntersection, MinEpsilon, MaxEpsilon)
    }

    //Set the verbosity level of Geant4 according to the logging settings of the module
    int g4VerboseLevel = 0;
    switch (LogSystem::Instance().getCurrentLogLevel()) {
      case LogConfig::c_Debug : g4VerboseLevel = 2;
        break;
      case LogConfig::c_Info  : g4VerboseLevel = 1;
        break;
      default: g4VerboseLevel = 0;
    }
    G4EventManager::GetEventManager()->SetVerboseLevel(g4VerboseLevel);
    G4RunManager::GetRunManager()->SetVerboseLevel(g4VerboseLevel);
    G4EventManager::GetEventManager()->GetTrackingManager()->SetVerboseLevel(
      trackingVerbosity); //turned out to be more useful as a parameter.
    G4LossTableManager::Instance()->SetVerbose(g4VerboseLevel);

    if (enableVisualization) {
      m_VisManager = new G4VisExecutive;
      m_VisManager->Initialize();
    }

    //Apply the Geant4 UI commands
    if (uiCommands.size() > 0) {
      G4UImanager* uiManager = G4UImanager::GetUIpointer();
      for (std::vector<std::string>::const_iterator iter = uiCommands.begin(); iter != uiCommands.end(); ++iter) {
        uiManager->ApplyCommand(*iter);
      }
    }

    // Construct the detector and bare-bones physics list
    myRunMgr.SetUserInitialization(new DetectorConstruction());
    G4Region* region = (*(G4RegionStore::GetInstance()))[0];
    region->SetProductionCuts(G4ProductionCutsTable::GetProductionCutsTable()->GetDefaultProductionCuts());
    myRunMgr.SetUserInitialization(new Simulation::ExtPhysicsList());
    myRunMgr.Initialize();
    B2INFO("ExtManager::Initialize(): Perform Geant4 final initialization: Geometry optimization, ExtPhysicsList calculations...");
    myRunMgr.beginRun(0);
    B2INFO("ExtManager::Initialize(): done, Geant4 ready (for extrapolation only)");
    G4StateManager::GetStateManager()->SetNewState(G4State_Idle);
  }

  // Prepare for the possibility of back-extrapolation
  if (m_MagneticField != NULL) {
    m_StdStepper = const_cast<G4MagIntegratorStepper*>(fieldManager->GetChordFinder()->GetIntegrationDriver()->GetStepper());
    m_ForwardEquationOfMotion = m_StdStepper->GetEquationOfMotion();
    m_BackwardEquationOfMotion = new G4ErrorMag_UsualEqRhs(m_MagneticField);
  }

  G4ErrorPropagatorData::GetErrorPropagatorData()->SetState(G4ErrorState_Init);
  if (status == 0) {
    B2INFO("ExtManager::Initialize(): " << caller << " will run without FullSim and has initialized the extrapolator");
  } else {
    B2INFO("ExtManager::Initialize(): " << caller << " will run with FullSim and has initialized the extrapolator");
  }
  return;
}

