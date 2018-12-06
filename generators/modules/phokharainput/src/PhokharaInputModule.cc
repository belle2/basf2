/************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014  Belle II Collaboration                              *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Torben Ferber                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <generators/modules/phokharainput/PhokharaInputModule.h>

#include <framework/utilities/FileSystem.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/GearDir.h>

#include <TLorentzVector.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PhokharaInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
PhokharaInputModule::PhokharaInputModule() : Module(), m_initial(BeamParameters::c_smearVertex)
{
  //Set module properties
  setDescription("Generates radiative return events with PHOKHARA.");

  //Parameter definition
  addParam("FinalState", m_finalState,
           "Final state: mu+mu-(0, default), pi+pi-(1), 2pi0pi+pi-(2), 2pi+2pi-(3), ppbar(4), nnbar(5), K+K-(6), K0K0bar(7), pi+pi-pi0(8), lamb(->pi-p)lambbar(->pi+pbar)(9), eta pi+ pi- (10)",
           1);
  addParam("ReplaceMuonsByVirtualPhoton", m_replaceMuonsByVirtualPhoton,
           "Replace muons by a virtual photon (for FinalState == 0 only).",
           false);
  addParam("SearchMax", m_nSearchMax, "Number of events used to search for maximum of differential cross section", 100000);
  addParam("Epsilon", m_epsilon, "Soft/hard photon separator", 0.0001);
  addParam("nMaxTrials", m_nMaxTrials, "Maximum trials per event", 10000);

  addParam("LO", m_LO, "LO: 1ph(0, default), Born: 0ph(1), only Born: 0ph(-1)", 0);
  addParam("NLO", m_NLO, "NLO, for 1ph only: off(0, default), on(1)", 0);
  addParam("QED", m_QED, "ISR only(0, default), ISR+FSR(1), ISR+INT+FSR(2)", 0);
  addParam("NLOIFI", m_NLOIFI, "IFI, NLO only: off(0, default), on(1)", 0);
  addParam("Alpha", m_alpha, "Vacuum polarization switch: off (0), on (1,[by Fred Jegerlehner], default), on (2,[by Thomas Teubner])",
           1);
  addParam("PionFF", m_pionff, "Pion FF: KS PionFormFactor(0, default), GS old 1), GS new(2)", 0);
  addParam("PionStructure", m_pionstructure,
           "For pi+pi- only: f0+f0(600): K+K- model(0, default), no structure model(1), no f0+f0(600)(2), f0 KLOE(3)", 0);
  addParam("KaonFF", m_kaonff,
           "Kaon FF: KaonFormFactor constrained(0, default), KaonFormFactor unconstrained(1) KaonFormFactor old(2)", 0);
  addParam("NarrowRes", m_narres, "Only for m_finalState = 0,1,6,7: No narrow resonances (0, default), J/Psi (1) and Psi(2S) (2)", 0);
  addParam("ProtonFF", m_protonff, "ProtonFormFactor old(0), ProtonFormFactor new(1)", 1);

  addParam("ScatteringAngleRangePhoton", m_ScatteringAngleRangePhoton,
           "Min [0] and Max [1] value for the scattering angle of photons [deg], default (0, 180)", make_vector(0.0, 180.0));
  addParam("ScatteringAngleRangeFinalStates", m_ScatteringAngleRangeFinalStates,
           "Min [0] and Max [1] value for the scattering angle of pions(muons,nucleons,kaons) [deg], default (0, 180)", make_vector(0.0,
               180.0));

  addParam("MinInvMassHadronsGamma", m_MinInvMassHadronsGamma, "Minimal hadrons/muons-gamma invariant mass squared [GeV^2]", 0.0);
  addParam("MinInvMassHadrons", m_MinInvMassHadrons, "Minimal hadrons/muons invariant mass squared [GeV^2]", 0.2);
  addParam("ForceMinInvMassHadronsCut", m_ForceMinInvMassHadronsCut,
           "Force application of the MinInvMassHadrons cut "
           "It is ignored by PHOKHARA with LO = 1, NLO = 1.",
           false);
  addParam("MaxInvMassHadrons", m_MaxInvMassHadrons, "Maximal hadrons/muons invariant mass squared [GeV^2]", 112.0);
  addParam("MinEnergyGamma", m_MinEnergyGamma, "Minimal photon energy/missing energy, must be greater than 0.0098 * CMS energy [GeV]",
           0.15);

  addParam("ParameterFile", m_ParameterFile, "File that contain particle properties",
           FileSystem::findFile("/data/generators/phokhara/const_and_model_paramall9.1.dat"));
//   addParam("InputFile", m_InputFile, "File that contain input configuration", FileSystem::findFile("/data/generators/phokhara/input_9.1.dat"));

}

//-----------------------------------------------------------------
//                 Destructor
//-----------------------------------------------------------------
PhokharaInputModule::~PhokharaInputModule()
{

}

//-----------------------------------------------------------------
//                 Initialize
//-----------------------------------------------------------------
void PhokharaInputModule::initialize()
{
  StoreArray<MCParticle> mcparticle;
  mcparticle.registerInDataStore();

  if (m_replaceMuonsByVirtualPhoton) {
    if (m_QED != 0) {
      B2FATAL("You requested to replace muons by a virtual photon. In this "
              "mode, PHOKHARA works as an ISR generator. The parameter QED "
              "should be set to 0 (ISR only). If FSR is taken into account "
              "(QED = 1 or 2), the results will be incorrect.");
    }
    if (m_NLOIFI != 0) {
      B2FATAL("You requested to replace muons by a virtual photon. In this "
              "mode, PHOKHARA works as an ISR generator. The parameter NLOIFI "
              "should be set to 0 (off). If simultaneous emission of initial "
              "and final-state photons is taken into account (NLOIFI = 1), "
              "the results will be incorrect.");
    }
  }
  //Beam Parameters, initial particle - PHOKHARA cannot handle beam energy spread
  m_initial.initialize();

}

//-----------------------------------------------------------------
//                 Event
//-----------------------------------------------------------------
void PhokharaInputModule::event()
{

  // Check if the BeamParameters have changed (if they do, abort the job! otherwise cross section calculation will be a nightmare.)
  if (m_beamParams.hasChanged()) {
    if (!m_initialized) {
      initializeGenerator();
    } else {
      B2FATAL("PhokharaInputModule::event(): BeamParameters have changed within a job, this is not supported for PHOKHARA!");
    }
  }

  // initial particle from beam parameters
  MCInitialParticles& initial = m_initial.generate();

  // true boost
  TLorentzRotation boost = initial.getCMSToLab();

  // vertex
  TVector3 vertex = initial.getVertex();

  m_mcGraph.clear();
  m_generator.generateEvent(m_mcGraph, vertex, boost);
  m_mcGraph.generateList("", MCParticleGraph::c_setDecayInfo | MCParticleGraph::c_checkCyclic);

}


//-----------------------------------------------------------------
//                 Terminate
//-----------------------------------------------------------------
void PhokharaInputModule::terminate()
{

  m_generator.term();
}

void PhokharaInputModule::initializeGenerator()
{

  const BeamParameters& nominal = m_initial.getBeamParameters();
  m_cmsEnergy = nominal.getMass();

  m_generator.setCMSEnergy(m_cmsEnergy);

  m_generator.setNSearchMax(m_nSearchMax);
  m_generator.setFinalState(m_finalState);
  m_generator.setReplaceMuonsByVirtualPhoton(m_replaceMuonsByVirtualPhoton);
  m_generator.setNMaxTrials(m_nMaxTrials);
  m_generator.setEpsilon(m_epsilon);
  m_generator.setLO(m_LO);
  m_generator.setNLO(m_NLO);
  m_generator.setQED(m_QED);
  m_generator.setNLOIFI(m_NLOIFI);
  m_generator.setAlpha(m_alpha);
  m_generator.setPionFF(m_pionff);
  m_generator.setKaonFF(m_kaonff);
  m_generator.setPionStructure(m_pionstructure);
  m_generator.setNarrowRes(m_narres);
  m_generator.setProtonFF(m_protonff);

  m_generator.setMinInvMassHadronsGamma(m_MinInvMassHadronsGamma);
  m_generator.setm_MinInvMassHadrons(m_MinInvMassHadrons);
  m_generator.setForceMinInvMassHadronsCut(m_ForceMinInvMassHadronsCut);
  m_generator.setm_MaxInvMassHadrons(m_MaxInvMassHadrons);
  m_generator.setMinEnergyGamma(m_MinEnergyGamma);
  m_generator.setScatteringAngleRangePhoton(vectorToPair(m_ScatteringAngleRangePhoton, "ScatteringAngleRangePhoton"));
  m_generator.setScatteringAngleRangeFinalStates(vectorToPair(m_ScatteringAngleRangeFinalStates, "ScatteringAngleRangeFinalStates"));

  m_generator.init(m_ParameterFile);

  m_initialized = true;

}
