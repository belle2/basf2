/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Guofu Cao                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/modules/fullsim/FullSimModule.h>
#include <simulation/kernel/RunManager.h>
#include <simulation/kernel/DetectorConstruction.h>
#include <simulation/kernel/PhysicsList.h>
#include <simulation/kernel/MagneticField.h>
#include <simulation/kernel/PrimaryGeneratorAction.h>
#include <simulation/kernel/EventAction.h>
#include <simulation/kernel/TrackingAction.h>
#include <simulation/kernel/SteppingAction.h>
#include <simulation/kernel/StackingAction.h>

#include <generators/dataobjects/MCParticle.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/gearbox/Unit.h>

#include <G4TransportationManager.hh>
#include <G4Transportation.hh>
#include <G4ParticleTable.hh>
#include <G4DecayTable.hh>
#include <G4EventManager.hh>
#include <G4RunManager.hh>
#include <G4UImanager.hh>
#include <G4UIExecutive.hh>
#include <G4VisExecutive.hh>
#include <G4StepLimiter.hh>
#include <G4LossTableManager.hh>
#include <G4HadronicProcessStore.hh>

#include <G4Mag_UsualEqRhs.hh>
#include <G4NystromRK4.hh>
#include <G4HelixExplicitEuler.hh>
#include <G4HelixSimpleRunge.hh>
#include <G4CachedMagneticField.hh>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(FullSim)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

FullSimModule::FullSimModule() : Module(), m_visManager(NULL)
{
  //Set module properties and the description
  setDescription("Performs the full Geant4 detector simulation. Requires a valid geometry in memory.");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  //Parameter definition
  addParam("InputMCParticleCollection", m_mcParticleInputColName, "The name of the input MCParticle collection.", string(""));
  addParam("OutputMCParticleCollection", m_mcParticleOutputColName, "The name of the output MCParticle collection.", string(""));
  addParam("ThresholdImportantEnergy", m_thresholdImportantEnergy, "[GeV] A particle which got 'stuck' and has less than this energy will be killed after 'ThresholdTrials' trials.", 0.250);
  addParam("ThresholdTrials", m_thresholdTrials, "Geant4 will try 'ThresholdTrials' times to move a particle which got 'stuck' and has an energy less than 'ThresholdImportantEnergy'.", 10);
  addParam("TrackingVerbosity", m_trackingVerbosity, "Tracking verbosity: 0=Silent; 1=Min info per step; 2=sec particles; 3=pre/post step info; 4=like 3 but more info; 5=proposed step length info.", 0);
  addParam("HadronProcessVerbosity", m_hadronprocessVerbosity, "Hadron Process verbosity: 0=Silent; 1=info level; 2=debug level", 0);
  addParam("PhysicsList", m_physicsList, "The name of the physics list which is used for the simulation.", string("FTFP_BERT"));
  addParam("RegisterOptics", m_optics, "If true, G4OpticalPhysics is registered in Geant4 PhysicsList.", true);
  addParam("ProductionCut", m_productionCut, "[cm] Apply continuous energy loss to primary particle which has no longer enough energy to produce secondaries which travel at least the specified productionCut distance.", 0.07);
  addParam("MaxNumberSteps", m_maxNumberSteps, "The maximum number of steps before the track transportation is stopped and the track is killed.", 100000);
  addParam("PhotonFraction", m_photonFraction, "The fraction of Cerenkov photons which will be kept and propagated.", 0.3);
  addParam("EnableVisualization", m_EnableVisualization, "If set to True the Geant4 visualization support is enabled.", false);
  addParam("StoreOpticalPhotons", m_storeOpticalPhotons, "If set to True optical photons are stored in MCParticles", false);
  addParam("StoreAllSecondaries", m_storeSecondaries, "If set to True all secondaries produced by Geant over a kinetic energy cut are stored in MCParticles, otherwise do not store them", false);
  addParam("SecondariesEnergyCut", m_energyCut, "[MeV] Kinetic energy cut for storing secondaries", 1.0);
  addParam("magneticField", m_magneticField, "Chooses the magnetic field stepper used by Geant4. possible values are: default, nystrom, expliciteuler, simplerunge", string("default"));
  addParam("magneticCacheDistance", m_magneticCacheDistance, "Minimum distance for BField lookup in cm. If the next requested point is closer than this distance than return the flast BField value. 0 means no caching", 0.0);

  vector<string> defaultCommands;
  addParam("UICommands", m_uiCommands, "A list of Geant4 UI commands that should be applied before the simulation starts.", defaultCommands);

  //Make sure the instance of the run manager is created now to initialize some stuff we need for geometry
  RunManager::Instance();
}


FullSimModule::~FullSimModule()
{

}


void FullSimModule::initialize()
{
  //Register the collections we want to use
  StoreArray<MCParticle>::required(m_mcParticleInputColName);
  StoreObjPtr<EventMetaData>::required();
  StoreArray<MCParticle>::registerPersistent(m_mcParticleOutputColName);

  //Get the instance of the run manager.
  RunManager& runManager = RunManager::Instance();

  //Add Geometry
  runManager.SetUserInitialization(new DetectorConstruction());

  //Create the Physics list
  PhysicsList* physicsList = new PhysicsList(m_physicsList);
  physicsList->setProductionCutValue(m_productionCut);
  if (m_optics) physicsList->registerOpticalPhysicsList();
  runManager.SetUserInitialization(physicsList);

  //Create the magnetic field for the Geant4 simulation
  if (m_magneticField != "none") {
    G4MagneticField* magneticField = new MagneticField();
    if (m_magneticCacheDistance > 0) {
      magneticField = new G4CachedMagneticField(magneticField, m_magneticCacheDistance);
    }
    G4FieldManager* fieldManager = G4TransportationManager::GetTransportationManager()->GetFieldManager();
    fieldManager->SetDetectorField(magneticField);
    if (m_magneticField != "default") {
      //We only use Magnetic field so let's try the specialized steppers
      G4Mag_UsualEqRhs* pMagFldEquation = new G4Mag_UsualEqRhs(magneticField);
      G4MagIntegratorStepper* stepper(0);
      if (m_magneticField == "nystrom") {
        stepper = new G4NystromRK4(pMagFldEquation);
      } else if (m_magneticField == "expliciteuler") {
        stepper = new G4HelixExplicitEuler(pMagFldEquation);
      } else if (m_magneticField == "simplerunge") {
        stepper = new G4HelixSimpleRunge(pMagFldEquation);
      } else {
        B2FATAL("Unknown magnetic field option: " << m_magneticField);
      }
      //Set a minimum stepsize: The chordfinder should not attempt to limit
      //the stepsize to something less than 10µm (which is the default
      //value of Geant4
      G4ChordFinder* chordfinder = new G4ChordFinder(magneticField, 1e-2 * mm, stepper);
      fieldManager->SetChordFinder(chordfinder);
    } else {
      fieldManager->CreateChordFinder(magneticField);
    }
    //This might be a good place to optimize the Integration parameters (DeltaOneStep, DeltaIntersection, MinEpsilon, MaxEpsilon)
  }

  //Create the generator action which takes the MCParticle list and converts it to Geant4 primary vertices.
  G4VUserPrimaryGeneratorAction* generatorAction = new PrimaryGeneratorAction(m_mcParticleInputColName, m_mcParticleGraph);
  runManager.SetUserAction(generatorAction);

  //Add the event action which creates the final MCParticle list and the Relation list.
  EventAction* eventAction = new EventAction(m_mcParticleOutputColName, m_mcParticleGraph);
  runManager.SetUserAction(eventAction);

  //Add the tracking action which handles the secondary particles created by Geant4.
  TrackingAction* trackingAction = new TrackingAction(m_mcParticleGraph);
  trackingAction->setIgnoreOpticalPhotons(!m_storeOpticalPhotons);
  trackingAction->setIgnoreSecondaries(!m_storeSecondaries);
  trackingAction->setKineticEnergyCut(m_energyCut);
  runManager.SetUserAction(trackingAction);

  //Add the stepping action which provides additional security checks
  SteppingAction* steppingAction = new SteppingAction();
  steppingAction->setMaxNumberSteps(m_maxNumberSteps);
  runManager.SetUserAction(steppingAction);

  //Add the stacking action which provides performance speed ups for the handling of optical photons
  StackingAction* stackingAction = new StackingAction();
  stackingAction->setPropagatedPhotonFraction(m_photonFraction);
  runManager.SetUserAction(stackingAction);

  //Initialize G4 kernel
  runManager.Initialize();

  //Set the parameters for the G4Transportation system.
  //To make sure we really change all G4Transportation classes, we loop over all particles
  //even if the pointer to the G4Transportation object seems to be the same for all particles.
  G4ParticleTable::G4PTblDicIterator* partIter = G4ParticleTable::GetParticleTable()->GetIterator();
  partIter->reset();
  while ((*partIter)()) {
    G4ParticleDefinition* currParticle = partIter->value();
    G4ProcessVector& currProcList = *currParticle->GetProcessManager()->GetProcessList();
    for (int iProcess = 0; iProcess < currProcList.size(); ++iProcess) {
      G4Transportation* transport = dynamic_cast<G4Transportation*>(currProcList[iProcess]);
      if (transport != NULL) {
        transport->SetThresholdImportantEnergy(m_thresholdImportantEnergy / Unit::MeV); //Geant4 energy unit is MeV
        transport->SetThresholdTrials(m_thresholdTrials);
        break;
      }
    }
    // Add StepLimiter process for charged tracks.
    double zeroChargeTol = 0.01 * Unit::e;
    if (fabs(currParticle->GetPDGCharge()) > zeroChargeTol) {
      currParticle->GetProcessManager()->AddDiscreteProcess(new G4StepLimiter());
      B2DEBUG(100, "Added StepLimiter process for " << currParticle->GetParticleName())
    }
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
  G4EventManager::GetEventManager()->GetTrackingManager()->SetVerboseLevel(m_trackingVerbosity); //turned out to be more useful as a parameter.
  //G4HadronicProcessStore::Instance()->SetVerbose(g4VerboseLevel);
  G4HadronicProcessStore::Instance()->SetVerbose(m_hadronprocessVerbosity);
  G4LossTableManager::Instance()->SetVerbose(g4VerboseLevel);


  if (m_EnableVisualization) {
    m_visManager = new G4VisExecutive;
    m_visManager->Initialize();
  }

  //Apply the Geant4 UI commands
  if (m_uiCommands.size() > 0) {
    G4UImanager* uiManager = G4UImanager::GetUIpointer();
    for (vector<string>::iterator iter = m_uiCommands.begin(); iter != m_uiCommands.end(); iter++) {
      uiManager->ApplyCommand(*iter);
    }
  }
}


void FullSimModule::beginRun()
{
  //Get the event meta data
  StoreObjPtr<EventMetaData> eventMetaDataPtr;

  //Begin the Geant4 run
  RunManager::Instance().beginRun(eventMetaDataPtr->getRun());
}


void FullSimModule::event()
{
  //Get the event meta data
  StoreObjPtr<EventMetaData> eventMetaDataPtr;

  //Process the event
  RunManager::Instance().processEvent(eventMetaDataPtr->getEvent());
}


void FullSimModule::endRun()
{
  //Terminate the Geant4 run
  RunManager::Instance().endRun();

  //G4UIExecutive * ui = new G4UIExecutive(0,0);
  //ui->SessionStart();
  //delete ui;
}


void FullSimModule::terminate()
{
  if (m_visManager != NULL) delete m_visManager;
  RunManager::Instance().destroy();
}
