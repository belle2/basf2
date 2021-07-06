/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/modules/phokharainput/PhokharaInputModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/utilities/FileSystem.h>
#include <framework/utilities/IOIntercept.h>

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
  setDescription("Generates radiative return events with PHOKHARA 10.");

  //Parameter definition
  addParam("FinalState", m_finalState,
           "Final state:  mu+mu-(0), pi+pi-(1), 2pi0pi+pi-(2), 2pi+2pi-(3), ppbar(4), nnbar(5), K+K-(6), K0K0bar(7), pi+pi-pi0(8), lamb(->pi-p)lambbar(->pi+pbar)(9), eta pi+ pi- (10), chi_c1 production (11), chi_c2 production (12), pi0 g (13), eta g (14), etaP g (15)",
           1);
  addParam("ReplaceMuonsByVirtualPhoton", m_replaceMuonsByVirtualPhoton,
           "Replace muons by a virtual photon (for FinalState == 0 only).",
           false);
  addParam("SearchMax", m_nSearchMax, "Number of events used to search for maximum of differential cross section", 100000);
  addParam("Epsilon", m_epsilon, "Soft/hard photon separator", 0.0001);
  addParam("nMaxTrials", m_nMaxTrials, "Maximum trials per event", 10000);
  addParam("Weighted", m_weighted, "generate weighted events", 0);

  addParam("LO", m_LO, "ph0  Born: 1ph(0), Born: 0ph(1), only Born: 0ph(-1)", 0);
  addParam("NLO", m_NLO, "1 photon : Born(0), NLO(1)", 0);
  addParam("FullNLO", m_fullNLO, "full NLO : No(0), Yes(1). Allowed only for ph0=1, nlo=1, fsr=2, fsrnlo=1", 0);
  addParam("QED", m_QED, "ISR only(0, default), ISR+FSR(1), ISR+INT+FSR(2)", 0);
  addParam("IFSNLO", m_IFSNLO, "IFSNLO: no(0), yes(1)", 0);
  addParam("Alpha", m_alpha, "Vacuum polarization switch: off (0), on (1,[by Fred Jegerlehner], default), on (2,[by Thomas Teubner])",
           1);
  addParam("PionFF", m_pionff, "Pion FF: KS PionFormFactor(0, default), GS old 1), GS new(2)", 0);
  addParam("PionStructure", m_pionstructure,
           "For pi+pi- only: f0+f0(600): K+K- model(0, default), no structure model(1), no f0+f0(600)(2), f0 KLOE(3)", 0);
  addParam("KaonFF", m_kaonff,
           "Kaon FF: KaonFormFactor constrained(0, default), KaonFormFactor unconstrained(1) KaonFormFactor old(2)", 0);
  addParam("NarrowRes", m_narres, "Only for m_finalState = 0,1,6,7: No narrow resonances (0, default), J/Psi (1) and Psi(2S) (2)", 0);
  addParam("ProtonFF", m_protonff, "ProtonFormFactor old(0), ProtonFormFactor new(1)", 1);
  addParam("ChiSW", m_chi_sw,
           "chi_sw: Radiative return(0), Chi production(1), Radiative return + Chi production (2). Works only for pion=11 and pion=12.", 0);
  addParam("SwitchBeamResolution", m_be_r,
           "be_r: without beam resolution(0), with beam resolution(1). Works only for pion=11 and pion=12; (0) assumes exact CMS-Energy, (1) each beam ennergy (=CMS-Energy/2) is smeared with Gaussian distribution of the given variance=BeamResolution**2.",
           0);
  addParam("BeamResolution", m_beamres, "beamres - beam resolution for pion==11 and pion==12 only", 0.);

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

  const std::string defaultParameterFile =
    FileSystem::findFile("/data/generators/phokhara/const_and_model_paramall10.0.dat");
  addParam("ParameterFile", m_ParameterFile,
           "File that contains particle properties.",
           defaultParameterFile);
  addParam("BeamEnergySpread", m_BeamEnergySpread,
           "Simulate beam-energy spread (initializes PHOKHARA for every "
           "event - very slow).", false);
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
    if (m_finalState != 0) {
      B2FATAL("You requested to replace muons by a virtual photon, but the "
              "final state is not mu+ mu-.");
    }
    if (m_QED != 0) {
      B2FATAL("You requested to replace muons by a virtual photon. In this "
              "mode, PHOKHARA works as an ISR generator. The parameter QED "
              "should be set to 0 (ISR only). If FSR is taken into account "
              "(QED = 1 or 2), the results will be incorrect.");
    }
    if (m_IFSNLO != 0) {
      B2FATAL("You requested to replace muons by a virtual photon. In this "
              "mode, PHOKHARA works as an ISR generator. The parameter IFSNLO "
              "should be set to 0 (off). If simultaneous emission of initial "
              "and final-state photons is taken into account (IFSNLO = 1), "
              "the results will be incorrect.");
    }
  }
  if (m_finalState == 11 || m_finalState == 12) {
    if (m_narres != 1 || m_LO != 0 || m_NLO != 0) {
      B2FATAL("Generation of the chi_c1 or chi_c2 (final states 11 and 12, "
              "respectively) requires to turn on the J/psi (NarrowRes = 1) "
              "and use LO radiative-return mode (LO = 0, NLO = 0)");
    }
  }
  //Beam Parameters, initial particle - PHOKHARA cannot handle beam energy spread
  if (m_BeamEnergySpread) {
    m_initial.setAllowedFlags(BeamParameters::c_smearVertex |
                              BeamParameters::c_smearBeam);
  }
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

  StoreObjPtr<EventMetaData> evtMetaData("EventMetaData", DataStore::c_Event);

  // initial particle from beam parameters
  const MCInitialParticles& initial = m_initial.generate();

  // true boost
  TLorentzRotation boost = initial.getCMSToLab();

  // vertex
  TVector3 vertex = initial.getVertex();

  m_mcGraph.clear();
  if (m_BeamEnergySpread) {
    // PHOKHARA does not support beam-energy spread. To simulate it,
    // the generator is initialized with the new energy for every event.
    IOIntercept::OutputToLogMessages initLogCapture(
      "PHOKHARA", LogConfig::c_Debug, LogConfig::c_Debug, 100, 100);
    initLogCapture.start();
    m_generator.setCMSEnergy(initial.getMass());
    m_generator.init(m_ParameterFile);
    initLogCapture.finish();
  }
  double weight = m_generator.generateEvent(m_mcGraph, vertex, boost);
  m_mcGraph.generateList("", MCParticleGraph::c_setDecayInfo | MCParticleGraph::c_checkCyclic);

  //store the weight (1.0 for unweighted events)
  evtMetaData->setGeneratedWeight(weight);
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
  m_generator.setWeighted(m_weighted);
  m_generator.setFinalState(m_finalState);
  m_generator.setReplaceMuonsByVirtualPhoton(m_replaceMuonsByVirtualPhoton);
  m_generator.setNMaxTrials(m_nMaxTrials);
  m_generator.setEpsilon(m_epsilon);
  m_generator.setLO(m_LO);
  m_generator.setNLO(m_NLO);
  m_generator.setFullNLO(m_fullNLO);
  m_generator.setQED(m_QED);
  m_generator.setIFSNLO(m_IFSNLO);
  m_generator.setAlpha(m_alpha);
  m_generator.setPionFF(m_pionff);
  m_generator.setKaonFF(m_kaonff);
  m_generator.setPionStructure(m_pionstructure);
  m_generator.setNarrowRes(m_narres);
  m_generator.setProtonFF(m_protonff);
  m_generator.setChiSW(m_chi_sw);
  m_generator.setSwitchBeamResolution(m_be_r);
  m_generator.setBeamResolution(m_beamres);


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
