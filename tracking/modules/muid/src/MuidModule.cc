/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <tracking/modules/muid/MuidModule.h>

/* Tracking headers. */
#include <tracking/trackExtrapolateG4e/TrackExtrapolateG4e.h>

/* Belle 2 headers. */
#include <framework/logging/Logger.h>
#include <simulation/kernel/ExtManager.h>

/* CLHEP headers. */
#include <CLHEP/Units/SystemOfUnits.h>

/* Geant4 headers. */
#include <G4UImanager.hh>

/* C++ headers. */
#include <vector>

using namespace std;
using namespace Belle2;

REG_MODULE(Muid)

MuidModule::MuidModule() :
  Module(),
  m_MeanDt(0.0),
  m_MaxDt(0.0),
  m_MinPt(0.0),
  m_MinKE(0.0),
  m_MaxStep(0.0),
  m_MaxDistSqInVariances(0.0),
  m_MaxKLMTrackClusterDistance(0.0),
  m_MaxECLTrackClusterDistance(0.0),
  m_TrackingVerbosity(0),
  m_EnableVisualization(false),
  m_MagneticFieldStepperName(""),
  m_MagneticCacheDistance(0.0),
  m_DeltaChordInMagneticField(0.0)
{
  m_Extrapolator = TrackExtrapolateG4e::getInstance();
  m_PDGCodes.clear();
  m_Hypotheses.clear();
  m_UICommands.clear();
  setDescription("Identifies muons by extrapolating tracks from CDC to KLM using geant4e");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("pdgCodes", m_PDGCodes, "Positive-charge PDG codes for extrapolation hypotheses", m_PDGCodes);
  addParam("MeanDt", m_MeanDt, "[ns] Mean hit-trigger time for coincident hits (default 0)", double(0.0));
  // Raw KLM scintillator hit times are in the range from -5000 to -4000 ns
  // approximately. The time window can be readjusted after completion of
  // the implementation of KLM time calibration.
  addParam("MaxDt", m_MaxDt, "[ns] Coincidence window half-width for in-time KLM hits.", double(10000.0));
  addParam("MinPt", m_MinPt, "[GeV/c] Minimum transverse momentum of a particle that will be extrapolated (default 0.1)",
           double(0.1));
  addParam("MinKE", m_MinKE, "[GeV] Minimum kinetic energy of a particle to continue extrapolation (default 0.002)", double(0.002));
  addParam("MaxStep", m_MaxStep, "[cm] Maximum step size during extrapolation (use 0 for infinity; default 25)", double(25.0));
  addParam("MaxDistSigma", m_MaxDistSqInVariances, "[#sigmas] Maximum hit-to-extrapolation difference (default 3.5)", double(3.5));
  addParam("MaxKLMTrackClusterDistance", m_MaxKLMTrackClusterDistance,
           "[cm] Maximum distance of closest approach of track to KLM cluster for match (default 150)", double(150.0));
  addParam("MaxECLTrackClusterDistance", m_MaxECLTrackClusterDistance,
           "[cm] Maximum distance of closest approach of track to ECL cluster for match (default 100)", double(100.0));
  // Additional parameters copied from FullSimModule
  addParam("TrackingVerbosity", m_TrackingVerbosity,
           "Tracking verbosity: 0=Silent; 1=Min info per step; 2=sec particles; 3=pre/post step info; 4=like 3 but more info; 5=proposed step length info.",
           0);
  addParam("EnableVisualization", m_EnableVisualization, "If set to True the Geant4 visualization support is enabled.", false);
  addParam("magneticFieldStepper", m_MagneticFieldStepperName,
           "Chooses the magnetic field stepper used by Geant4. possible values are: default, nystrom, expliciteuler, simplerunge",
           string("default"));
  addParam("magneticCacheDistance", m_MagneticCacheDistance,
           "Minimum distance for BField lookup in cm. If the next requested point is closer than this distance than return the flast BField value. 0 means no caching",
           0.0);
  addParam("deltaChordInMagneticField", m_DeltaChordInMagneticField,
           "[mm] The maximum miss-distance between the trajectory curve and its linear cord(s) approximation", 0.25);
  addParam("addHitsToRecoTrack", m_addHitsToRecoTrack,
           "Parameter to add the found hits also to the reco tracks or not. Is turned off by default. "
           "Make sure to refit the track afterwards.",
           m_addHitsToRecoTrack);
  vector<string> defaultCommands;
  addParam("UICommands", m_UICommands, "A list of Geant4 UI commands that should be applied at the start of the job.",
           defaultCommands);
}

MuidModule::~MuidModule()
{
}

void MuidModule::initialize()
{
  // Initialize the (singleton) extrapolation manager.  It will check
  // whether it will run with or without FullSimModule and with or without
  // Ext (or any other geant4e-based extrapolation module).
  Simulation::ExtManager* extMgr = Simulation::ExtManager::GetManager();
  extMgr->Initialize("Muid", m_MagneticFieldStepperName, m_MagneticCacheDistance, m_DeltaChordInMagneticField,
                     m_EnableVisualization, m_TrackingVerbosity, m_UICommands);

  // Redefine geant4e step length, magnetic field step limitation (fraction of local curvature radius),
  // and kinetic energy loss limitation (maximum fractional energy loss) by communicating with
  // the geant4 UI.  (Commands were defined in ExtMessenger when physics list was set up.)
  // *NOTE* If module muid runs after this, its G4UImanager commands will override these.
  m_MaxStep = ((m_MaxStep == 0.0) ? 10.0 : std::min(10.0, m_MaxStep)) * CLHEP::cm;
  char stepSize[80];
  std::sprintf(stepSize, "/geant4e/limits/stepLength %8.2f mm", m_MaxStep);
  G4UImanager::GetUIpointer()->ApplyCommand(stepSize);
  G4UImanager::GetUIpointer()->ApplyCommand("/geant4e/limits/magField 0.001");
  G4UImanager::GetUIpointer()->ApplyCommand("/geant4e/limits/energyLoss 0.05");

  // Hypotheses for MUID extrapolation (default is muon only)
  if (m_PDGCodes.empty()) {
    m_Hypotheses.push_back(Const::muon);
  } else { // user defined - intended for debugging only!
    std::vector<Const::ChargedStable> stack;
    for (const Const::ChargedStable pdgIter : Const::chargedStableSet) {
      stack.push_back(pdgIter);
    }
    for (unsigned int i = 0; i < m_PDGCodes.size(); ++i) {
      for (unsigned int k = 0; k < stack.size(); ++k) {
        if (abs(m_PDGCodes[i]) == stack[k].getPDGCode()) {
          m_Hypotheses.push_back(stack[k]);
          stack.erase(stack.begin() + k);
          --k;
        }
      }
    }
    if (m_Hypotheses.empty()) B2ERROR("No valid PDG codes for extrapolation");
  }

  for (unsigned int i = 0; i < m_Hypotheses.size(); ++i) {
    B2INFO("Muid hypothesis for PDG code " << m_Hypotheses[i].getPDGCode() << " and its antiparticle will be extrapolated");
  }

  // Initialize the extrapolator engine for MUID (vs EXT)
  // *NOTE* that MinPt and MinKE are shared by MUID and EXT; only last caller wins
  m_Extrapolator->initialize(m_MeanDt, m_MaxDt, m_MaxDistSqInVariances, m_MaxKLMTrackClusterDistance,
                             m_MaxECLTrackClusterDistance, m_MinPt, m_MinKE, m_addHitsToRecoTrack, m_Hypotheses);
  return;
}

void MuidModule::beginRun()
{
  m_Extrapolator->beginRun(true);
}

void MuidModule::event()
{
  m_Extrapolator->event(true);
}

void MuidModule::endRun()
{
  m_Extrapolator->endRun(true);
}

void MuidModule::terminate()
{
  m_Extrapolator->terminate(true);
}
