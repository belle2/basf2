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
#include <tracking/modules/ext/ExtModule.h>

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

REG_MODULE(Ext)

ExtModule::ExtModule() :
  Module(),
  m_MinPt(0.0),
  m_MinKE(0.0),
  m_MaxStep(0.0),
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
  setDescription("Extrapolates tracks from CDC to outer detectors using geant4e");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("pdgCodes", m_PDGCodes, "Positive-charge PDG codes for extrapolation hypotheses", m_PDGCodes);
  addParam("MinPt", m_MinPt, "[GeV/c] Minimum transverse momentum of a particle that will be extrapolated (default 0.1)",
           double(0.1));
  addParam("MinKE", m_MinKE, "[GeV] Minimum kinetic energy of a particle to continue extrapolation (default 0.002)", double(0.002));
  addParam("MaxStep", m_MaxStep, "[cm] Maximum step size during extrapolation (use 0 for infinity) (default 25)", double(25.0));
  // Additional parameters copied from FullSimModule
  addParam("TrackingVerbosity", m_TrackingVerbosity,
           "Tracking verbosity: 0=Silent; 1=Min info per step; 2=sec particles; 3=pre/post step info; 4=like 3 but more info; 5=proposed step length info",
           0);
  addParam("EnableVisualization", m_EnableVisualization, "If set to True the Geant4 visualization support is enabled", false);
  addParam("magneticFieldStepper", m_MagneticFieldStepperName,
           "Chooses the magnetic field stepper used by Geant4. possible values are: default, nystrom, expliciteuler, simplerunge",
           string("default"));
  addParam("magneticCacheDistance", m_MagneticCacheDistance,
           "Minimum distance for BField lookup in cm. If the next requested point is closer than this distance than return the last BField value. 0 means no caching",
           0.0);
  addParam("deltaChordInMagneticField", m_DeltaChordInMagneticField,
           "[mm] The maximum miss-distance between the trajectory curve and its linear cord(s) approximation", 0.25);
  vector<string> defaultCommands;
  addParam("UICommands", m_UICommands, "A list of Geant4 UI commands that should be applied at the start of the job",
           defaultCommands);
}

ExtModule::~ExtModule()
{
}

void ExtModule::initialize()
{
  // Initialize the (singleton) extrapolation manager.  It will check
  // whether it will run with or without FullSimModule and with or without
  // Muid (or any other geant4e-based extrapolation module).
  Simulation::ExtManager* extMgr = Simulation::ExtManager::GetManager();
  extMgr->Initialize("Ext", m_MagneticFieldStepperName, m_MagneticCacheDistance, m_DeltaChordInMagneticField,
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

  // Hypotheses for EXT extrapolation (default is every particle in
  // Const::chargedStableSet, i.e., electron, muon, pion, kaon, proton, deuteron)
  m_Hypotheses.clear();
  if (m_PDGCodes.empty()) {
    for (const Const::ChargedStable pdgIter : Const::chargedStableSet) {
      m_Hypotheses.push_back(pdgIter);
    }
  } else { // user defined
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
    B2INFO("Ext hypothesis for PDG code " << m_Hypotheses[i].getPDGCode() << " and its antiparticle will be extrapolated");
  }

  // Initialize the extrapolator engine for EXT (vs MUID)
  // *NOTE* that MinPt and MinKE are shared by MUID and EXT; only last caller wins
  m_Extrapolator->initialize(m_MinPt, m_MinKE, m_Hypotheses);
}

void ExtModule::beginRun()
{
  m_Extrapolator->beginRun(false);
}

void ExtModule::event()
{
  m_Extrapolator->event(false);
}

void ExtModule::endRun()
{
  m_Extrapolator->endRun(false);
}

void ExtModule::terminate()
{
  m_Extrapolator->terminate(false);
}
