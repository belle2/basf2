/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2014 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Guofu Cao, Martin Ritter                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/modules/fullsim/FullSimModule.h>
#include <simulation/kernel/RunManager.h>
#include <simulation/kernel/DetectorConstruction.h>
//- #include <simulation/kernel/PhysicsList.h>
#include <simulation/kernel/ExtPhysicsConstructor.h>
#include <simulation/kernel/MagneticField.h>
#include <simulation/kernel/PrimaryGeneratorAction.h>
#include <simulation/kernel/EventAction.h>
#include <simulation/kernel/TrackingAction.h>
#include <simulation/kernel/SteppingAction.h>
#include <simulation/kernel/StackingAction.h>

#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/gearbox/Unit.h>

#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>

#include <simulation/monopoles/G4MonopolePhysics.h>

#include <G4TransportationManager.hh>
#include <G4Transportation.hh>
#include <G4VUserPhysicsList.hh>
#include <G4PhysListFactory.hh>
#include <G4ProcessVector.hh>
#include <G4OpticalPhysics.hh>
#include <G4ParticleDefinition.hh>
#include <G4ParticleTable.hh>
#include <G4DecayTable.hh>
#include <G4EventManager.hh>
#include <G4RunManager.hh>
#include <G4UImanager.hh>
#include <G4UIExecutive.hh>
#include <G4VisExecutive.hh>
#include <G4StepLimiter.hh>
#include <G4EmParameters.hh>
#include <G4HadronicProcessStore.hh>
#include <G4CascadeChannelTables.hh>
#include <G4CascadeChannel.hh>
#include <G4InuclParticleNames.hh>

#include <G4Mag_UsualEqRhs.hh>
#include <G4NystromRK4.hh>
#include <G4HelixExplicitEuler.hh>
#include <G4HelixSimpleRunge.hh>
#include <G4CachedMagneticField.hh>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;
using namespace Belle2::Monopoles;
using namespace G4InuclParticleNames;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(FullSim)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

FullSimModule::FullSimModule() : Module(), m_useNativeGeant4(true)
{
  //Set module properties and the description
  setDescription("Performs the full Geant4 detector simulation. Requires a valid geometry in memory.");
  setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);

  //Parameter definition
  addParam("InputMCParticleCollection", m_mcParticleInputColName, "The name of the input MCParticle collection.", string(""));
  addParam("ThresholdImportantEnergy", m_thresholdImportantEnergy,
           "[GeV] A particle which got 'stuck' and has less than this energy will be killed after 'ThresholdTrials' trials.", 0.250);
  addParam("ThresholdTrials", m_thresholdTrials,
           "Geant4 will try 'ThresholdTrials' times to move a particle which got 'stuck' and has an energy less than 'ThresholdImportantEnergy'.",
           10);
  addParam("RunEventVerbosity", m_runEventVerbosity, "Geant4 run/event verbosity: 0=silent; 1=info level; 2=debug level", 0);
  addParam("TrackingVerbosity", m_trackingVerbosity,
           "Tracking verbosity: 0=Silent; 1=Min info per step; 2=sec particles; 3=pre/post step info; 4=like 3 but more info; 5=proposed step length info.",
           0);
  addParam("HadronProcessVerbosity", m_hadronProcessVerbosity, "Hadron Process verbosity: 0=Silent; 1=info level; 2=debug level", 0);
  addParam("EmProcessVerbosity", m_emProcessVerbosity, "Em Process verbosity: 0=Silent; 1=info level; 2=debug level", 0);
  addParam("PhysicsList", m_physicsList, "The name of the physics list which is used for the simulation.", string("FTFP_BERT"));
  addParam("RegisterOptics", m_optics, "If true, G4OpticalPhysics is registered in Geant4 PhysicsList.", true);
  addParam("RegisterMonopoles", m_monopoles, "If set to true, G4MonopolePhysics is registered in Geant4 PhysicsList.", false);
  addParam("MonopoleMagCharge", m_monopoleMagneticCharge, "The value of monopole magnetic charge in units of e+.", 1.0);
  addParam("ProductionCut", m_productionCut,
           "[cm] Apply continuous energy loss to primary particle which has no longer enough energy to produce secondaries which travel at least the specified productionCut distance.",
           0.07);
  addParam("MaxNumberSteps", m_maxNumberSteps,
           "The maximum number of steps before the track transportation is stopped and the track is killed.", 100000);
  addParam("PhotonFraction", m_photonFraction, "The fraction of Cerenkov photons which will be kept and propagated.", 0.5);
  addParam("EnableVisualization", m_EnableVisualization, "If set to True, the Geant4 visualization support is enabled.", false);

  addParam("StoreOpticalPhotons", m_storeOpticalPhotons, "If set to True, optical photons are stored in MCParticles.", false);
  addParam("StoreAllSecondaries", m_storeSecondaries,
           "If set to True, all secondaries produced by Geant4 over a kinetic energy cut are stored in MCParticles. Otherwise do not store them.",
           false);
  addParam("SecondariesEnergyCut", m_secondariesEnergyCut, "[MeV] Kinetic energy cut for storing secondaries", 1.0);
  addParam("StoreBremsstrahlungPhotons", m_storeBremsstrahlungPhotons,
           "If set to True, store BremsstrahlungPhotons over a kinetic energy cut in MCParticles. Otherwise do not store them.", false);
  addParam("BremsstrahlungPhotonsEnergyCut", m_bremsstrahlungPhotonsEnergyCut,
           "[MeV] Kinetic energy cut for storing bremsstrahlung photons", 10.0);
  addParam("StorePairConversions", m_storePairConversions,
           "If set to True, store e+ or e- from pair conversions over a kinetic energy cut in MCParticles. Otherwise do not store them.",
           false);
  addParam("PairConversionsEnergyCut", m_pairConversionsEnergyCut,
           "[MeV] Kinetic energy cut for storing e+ or e- from pair conversions", 10.0);

  addParam("magneticField", m_magneticFieldName,
           "Chooses the magnetic field stepper used by Geant4. Possible values are: default, nystrom, expliciteuler, simplerunge",
           string("default"));
  addParam("magneticCacheDistance", m_magneticCacheDistance,
           "Minimum distance for BField lookup in cm. If the next requested point is closer than this distance than return the flast BField value. 0 means no caching",
           0.0);
  addParam("deltaChordInMagneticField", m_deltaChordInMagneticField,
           "[mm] The maximum miss-distance between the trajectory curve and its linear cord(s) approximation", 0.25);

  vector<string> defaultCommands;
  addParam("UICommands", m_uiCommands, "A list of Geant4 UI commands that should be applied before the simulation starts.",
           defaultCommands);

  addParam("trajectoryStore", m_trajectoryStore,
           "If non-zero save the full trajectory of 1=primary, 2=non-optical or 3=all particles", 0);
  addParam("trajectoryDistanceTolerance", m_trajectoryDistanceTolerance,
           "Maximum deviation from the real trajectory points when merging "
           "segments (in cm)", 5e-4);

  //Make sure the instance of the run manager is created now to initialize some stuff we need for geometry
  RunManager::Instance();
  m_magneticField = NULL;
  m_uncachedField = NULL;
  m_magFldEquation = NULL;
  m_stepper = NULL;
  m_chordFinder = NULL;
  m_visManager = NULL;
  m_stepLimiter = NULL;
}


FullSimModule::~FullSimModule()
{

}


void FullSimModule::initialize()
{
  //FullSim does not create a new collection.
  //For most of the cases, the module will reuse and update the existing one.

  //For a certain cases, the input MCParticles collection name may be empty.
  if (m_mcParticleInputColName.empty()) {
    StoreArray<MCParticle>().isOptional(m_mcParticleInputColName);
  } else {
    StoreArray<MCParticle>().isRequired(m_mcParticleInputColName);
  }

  //Make sure the EventMetaData collection already exists.
  StoreObjPtr<EventMetaData>().isRequired();

  //Get the instance of the run manager.
  RunManager& runManager = RunManager::Instance();

  //Add Geometry
  runManager.SetUserInitialization(new DetectorConstruction());

  //Create the Physics list
  //- PhysicsList* physicsList = new PhysicsList(m_physicsList);
  //- physicsList->setProductionCutValue(m_productionCut);
  //- if (m_optics) physicsList->registerOpticalPhysicsList();
  //- runManager.SetUserInitialization(physicsList);
  G4PhysListFactory physListFactory;
  physListFactory.SetVerbose(m_runEventVerbosity);
  G4VModularPhysicsList* physicsList = NULL;
  if (physListFactory.IsReferencePhysList(m_physicsList)) physicsList = physListFactory.GetReferencePhysList(m_physicsList);
  if (physicsList == NULL) B2FATAL("Could not load the physics list " << m_physicsList);
  physicsList->RegisterPhysics(new ExtPhysicsConstructor);
  if (m_optics) physicsList->RegisterPhysics(new G4OpticalPhysics);
  if (m_monopoles) {
    physicsList->RegisterPhysics(new G4MonopolePhysics(m_monopoleMagneticCharge));
  }
  physicsList->SetDefaultCutValue((m_productionCut / Unit::mm) * CLHEP::mm);  // default is 0.7 mm

  // LEP: For geant4e-specific particles, set a big step so that AlongStep computes
  // all the energy (as is done in G4ErrorPhysicsList)
  G4ParticleTable::G4PTblDicIterator* myParticleIterator = G4ParticleTable::GetParticleTable()->GetIterator();
  myParticleIterator->reset();
  while ((*myParticleIterator)()) {
    G4ParticleDefinition* particle = myParticleIterator->value();
    if (particle->GetParticleName().compare(0, 4, "g4e_") == 0) {
      physicsList->SetParticleCuts(1.0E+9 * CLHEP::cm, particle);
    }
  }
  runManager.SetUserInitialization(physicsList);


  //Create the magnetic field for the Geant4 simulation
  if (m_magneticFieldName != "none") {
    m_magneticField = new Belle2::Simulation::MagneticField();
    if (m_magneticCacheDistance > 0) {
      m_uncachedField = m_magneticField;
      m_magneticField = new G4CachedMagneticField(m_uncachedField, m_magneticCacheDistance);
    }
    G4FieldManager* fieldManager = G4TransportationManager::GetTransportationManager()->GetFieldManager();
    fieldManager->SetDetectorField(m_magneticField);
    if (m_magneticFieldName != "default") {

      //We only use Magnetic field so let's try the specialized steppers
      m_magFldEquation = new G4Mag_UsualEqRhs(m_magneticField);
      if (m_magneticFieldName == "nystrom") {
        m_stepper = new G4NystromRK4(m_magFldEquation);
      } else if (m_magneticFieldName == "expliciteuler") {
        m_stepper = new G4HelixExplicitEuler(m_magFldEquation);
      } else if (m_magneticFieldName == "simplerunge") {
        m_stepper = new G4HelixSimpleRunge(m_magFldEquation);
      } else {
        B2FATAL("Unknown magnetic field option: " << m_magneticFieldName);
      }

      //Set a minimum stepsize (stepMinimum): The chordfinder should not attempt to limit
      //the stepsize to something less than 10µm (which is the default value of Geant4).
      m_chordFinder = new G4ChordFinder(m_magneticField, 1e-2 * CLHEP::mm, m_stepper);
      fieldManager->SetChordFinder(m_chordFinder);
    } else {
      fieldManager->CreateChordFinder(m_magneticField);
    }

    //Change DeltaCord (the max. miss-distance between the trajectory curve and its linear chord(s) approximation, if asked.
    G4ChordFinder* chordFinder = fieldManager->GetChordFinder();
    B2DEBUG(1, "Geant4 default deltaChord = " << chordFinder->GetDeltaChord());
    chordFinder->SetDeltaChord(m_deltaChordInMagneticField * CLHEP::mm);
    B2DEBUG(1, "DeltaChord after reset = " << chordFinder->GetDeltaChord());

    //This might be a good place to optimize the Integration parameters (DeltaOneStep, DeltaIntersection, MinEpsilon, MaxEpsilon)
  }

  //Create the generator action which takes the MCParticle list and converts it to Geant4 primary vertices.
  G4VUserPrimaryGeneratorAction* generatorAction = new PrimaryGeneratorAction(m_mcParticleInputColName, m_mcParticleGraph);
  runManager.SetUserAction(generatorAction);

  //Add the event action which creates the final MCParticle list and the Relation list.
  //The output collection name will be always "MCParticles".
  EventAction* eventAction = new EventAction("", m_mcParticleGraph);
  runManager.SetUserAction(eventAction);

  //Add the tracking action which handles the secondary particles created by Geant4.
  TrackingAction* trackingAction = new TrackingAction(m_mcParticleGraph);
  trackingAction->setIgnoreOpticalPhotons(!m_storeOpticalPhotons);
  trackingAction->setIgnoreSecondaries(!m_storeSecondaries);
  trackingAction->setSecondariesEnergyCut(m_secondariesEnergyCut);
  trackingAction->setIgnoreBremsstrahlungPhotons(!m_storeBremsstrahlungPhotons);
  trackingAction->setBremsstrahlungPhotonsEnergyCut(m_bremsstrahlungPhotonsEnergyCut);
  trackingAction->setIgnorePairConversions(!m_storePairConversions);
  trackingAction->setPairConversionsEnergyCut(m_pairConversionsEnergyCut);

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
  //Only one instance of G4StepLimiter is needed: see G4StepLimiterBuilder(), for example.
  m_stepLimiter = new G4StepLimiter();
  G4ParticleTable::G4PTblDicIterator* partIter = G4ParticleTable::GetParticleTable()->GetIterator();
  partIter->reset();
  while ((*partIter)()) {
    G4ParticleDefinition* currParticle = partIter->value();
    G4ProcessVector& currProcList = *currParticle->GetProcessManager()->GetProcessList();
    for (int iProcess = 0; iProcess < currProcList.size(); ++iProcess) {
      G4Transportation* transport = dynamic_cast<G4Transportation*>(currProcList[iProcess]);
      if (transport != nullptr) {
        //Geant4 energy unit is MeV
        transport->SetThresholdImportantEnergy(m_thresholdImportantEnergy / Unit::MeV * CLHEP::MeV);
        transport->SetThresholdTrials(m_thresholdTrials);
        break;
      }
    }
    // Add StepLimiter process for charged tracks.
    double zeroChargeTol = 0.01 * Unit::e;
    if (fabs(currParticle->GetPDGCharge()) > zeroChargeTol) {
      currParticle->GetProcessManager()->AddDiscreteProcess(m_stepLimiter);
      B2DEBUG(100, "Added StepLimiter process for " << currParticle->GetParticleName());
    }
  }

  // Inactivate all secondary-generating processes for g4e particles. This comprises
  // Cerenkov and Scintillation that were inserted by G4OpticalPhysics and the
  // CaptureAtRest process for g4e anti-deuteron.
  partIter->reset();
  while ((*partIter)()) {
    G4ParticleDefinition* currParticle = partIter->value();
    if (currParticle->GetParticleName().compare(0, 4, "g4e_") == 0) {
      G4ProcessManager* processManager = currParticle->GetProcessManager();
      if (processManager) {
        G4ProcessVector* processList = processManager->GetProcessList();
        for (int i = 0; i < processList->size(); ++i) {
          if (((*processList)[i]->GetProcessName() == "Cerenkov") ||
              ((*processList)[i]->GetProcessName() == "Scintillation") ||
              ((*processList)[i]->GetProcessName() == "hFritiofCaptureAtRest")) {
            processManager->SetProcessActivation(i, false);
          }
        }
      }
    }
  }

  //Set the verbosity level of Geant4 according to the logging settings of the module
  //int g4VerboseLevel = 0;
  //switch (LogSystem::Instance().getCurrentLogLevel()) {
  //  case LogConfig::c_Debug : g4VerboseLevel = 2;
  //    break;
  //  case LogConfig::c_Info  : g4VerboseLevel = 1;
  //    break;
  //  default: g4VerboseLevel = 0;
  //}
  //G4EventManager::GetEventManager()->SetVerboseLevel(g4VerboseLevel);
  //G4RunManager::GetRunManager()->SetVerboseLevel(g4VerboseLevel);
  G4EventManager::GetEventManager()->SetVerboseLevel(m_runEventVerbosity);
  G4RunManager::GetRunManager()->SetVerboseLevel(m_runEventVerbosity);
  G4EventManager::GetEventManager()->GetTrackingManager()->SetVerboseLevel(
    m_trackingVerbosity); //turned out to be more useful as a parameter.
  G4HadronicProcessStore::Instance()->SetVerbose(m_hadronProcessVerbosity);
  G4EmParameters::Instance()->SetVerbose(m_emProcessVerbosity);


  if (m_EnableVisualization) {
    m_visManager = new G4VisExecutive;
    m_visManager->Initialize();
  }

  //Apply the Geant4 UI commands
  if (m_uiCommands.size() > 0) {
    G4UImanager* uiManager = G4UImanager::GetUIpointer();
    for (vector<string>::iterator iter = m_uiCommands.begin(); iter != m_uiCommands.end(); ++iter) {
      uiManager->ApplyCommand(*iter);
    }
  }

  //Store Trajectories?
  if (m_trajectoryStore) {
    trackingAction->setStoreTrajectories(m_trajectoryStore, m_trajectoryDistanceTolerance);
    steppingAction->setStoreTrajectories(true);
  }

  //Physics tables are build in run initialization. We have run independent
  //geometry at the moment so there is no need to do this in begin run. Instead
  //we use one Geant4 run for all Belle2 runs we might encounter. So let's do
  //run initialization now to save memory when doing parallel processing
  B2INFO("Perform Geant4 final initialization: Geometry optimization, PhysicsList calculations...");
  RunManager::Instance().beginRun(0);
  B2INFO("done, Geant4 ready");
  //Otherwise we could use a fake run to do this and move RunManager::beginRun
  //back to beginRun()
  //runManager.BeamOn(0);
}


void FullSimModule::beginRun()
{
  //Nothing to do: geometry and physics are run independent
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
  //Nothing to do: geometry and physics are run independent
}

void FullSimModule::terminate()
{
  //We used one Geant4 run for all Belle2 runs so end the geant4 run here
  RunManager::Instance().endRun();
  //And clean up the run manager
  if (m_visManager != nullptr) delete m_visManager;
  RunManager::Instance().destroy();
  // Delete the step limiter process
  delete m_stepLimiter;
  // Delete the objects associated with transport in magnetic field
  if (m_chordFinder) delete m_chordFinder;
  if (m_stepper) delete m_stepper;
  if (m_magFldEquation) delete m_magFldEquation;
  if (m_uncachedField) delete m_uncachedField;
  if (m_magneticField) delete m_magneticField;
}
