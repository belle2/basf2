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
#include <generators/utilities/cm2LabBoost.h> //REMOMVE SOON!

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
PhokharaInputModule::PhokharaInputModule() : Module()
{
  //Set module properties
  setDescription("Generates radiative return events with PHOKHARA.");

  //Parameter definition
  addParam("FinalState", m_finalState,
           "Final state: mu+mu-(0, default), pi+pi-(1), 2pi0pi+pi-(2), 2pi+2pi-(3), ppbar(4), nnbar(5), K+K-(6), K0K0bar(7), pi+pi-pi0(8), lamb(->pi-p)lambbar(->pi+pbar)(9), eta pi+ pi- (10)",
           1);
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

  addParam("CMSEnergy", m_cmsEnergy, " [GeV], Only for m_boostMode = 0: ", 10.580);
  addParam("BoostMode", m_boostMode, "Mode of the boost: no boost (0), Belle II (1, default), Belle (2)", 1);

  addParam("ScatteringAngleRangePhoton", m_ScatteringAngleRangePhoton,
           "Min [0] and Max [1] value for the scattering angle of photons [deg], default (0, 180)", make_vector(60.0, 120.0));
  addParam("ScatteringAngleRangeFinalStates", m_ScatteringAngleRangeFinalStates,
           "Min [0] and Max [1] value for the scattering angle of pions(muons,nucleons,kaons) [deg], default (0, 180)", make_vector(0.0,
               180.0));

  addParam("MinInvMassHadronsGamma", m_MinInvMassHadronsGamma, "Minimal hadrons/muons-gamma invariant mass squared [GeV^2]", 0.0);
  addParam("MinInvMassHadrons", m_MinInvMassHadrons, "Minimal hadrons/muons invariant mass squared [GeV^2]", 0.2);
  addParam("MaxInvMassHadrons", m_MaxInvMassHadrons, "Maximal hadrons/muons invariant mass squared [GeV^2]", 0.5);
  addParam("MinEnergyGamma", m_MinEnergyGamma, "Minimal photon energy/missing energy, >0.01 CMS energy [GeV]", 5.0);

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
  StoreArray<MCParticle>::registerPersistent();

  //Depending on the settings, use the Belle II or Belle boost
  double ecm = -1.; //center of mass energy, >0 if boost is set

  if (m_boostMode == 1) {
    GearDir ler("/Detector/SuperKEKB/LER/");
    GearDir her("/Detector/SuperKEKB/HER/");

    m_generator.setBoost(getBoost(her.getDouble("energy"), ler.getDouble("energy"),
                                  her.getDouble("angle") - ler.getDouble("angle"), her.getDouble("angle")));

    //get CMS energy
    ecm = getBeamEnergyCM(her.getDouble("energy"), ler.getDouble("energy"), her.getDouble("angle") - ler.getDouble("angle"));

  } else {
    if (m_boostMode == 2) {

      //electron and positron beam energies (magic numbers from Jeremy)
      double electronBeamEnergy = 7.998213; // [GeV]
      double positronBeamEnergy = 3.499218; // [GeV]
      double crossingAngle = 22.0; //[mrad]

      double pzP = sqrt(positronBeamEnergy * positronBeamEnergy - 0.000510998918 * 0.000510998918);
      double pE  = sqrt(electronBeamEnergy * electronBeamEnergy - 0.000510998918 * 0.000510998918);
      TLorentzVector boostVector(pE * sin(crossingAngle * 0.001), 0., pE * cos(crossingAngle * 0.001) - pzP,
                                 electronBeamEnergy + positronBeamEnergy);

      m_generator.setBoost(boostVector.BoostVector());

      //get CMS energy
      ecm = getBeamEnergyCM(electronBeamEnergy, positronBeamEnergy, crossingAngle);
    }
  }

  m_generator.enableBoost(m_boostMode > 0);

  //overwrite user setting if boost is enabled!
  if (m_boostMode) {
    if (m_cmsEnergy > 0.) { //user has set a cms energy... should not be
      B2WARNING("CM energy set manually, but boost mode enabled, resetting ECM to " << ecm);
      m_generator.setCMSEnergy(ecm);
    }
  } else {
    if (m_cmsEnergy < 0.) {
      B2FATAL("CM energy not set: " << m_cmsEnergy);
    }
    m_generator.setCMSEnergy(m_cmsEnergy);
  }

  m_generator.setNSearchMax(m_nSearchMax);
  m_generator.setFinalState(m_finalState);
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
  m_generator.setm_MaxInvMassHadrons(m_MaxInvMassHadrons);
  m_generator.setMinEnergyGamma(m_MinEnergyGamma);
  m_generator.setScatteringAngleRangePhoton(vectorToPair(m_ScatteringAngleRangePhoton, "ScatteringAngleRangePhoton"));
  m_generator.setScatteringAngleRangeFinalStates(vectorToPair(m_ScatteringAngleRangeFinalStates, "ScatteringAngleRangeFinalStates"));

  m_generator.init(m_ParameterFile);

}

//-----------------------------------------------------------------
//                 Event
//-----------------------------------------------------------------
void PhokharaInputModule::event()
{
  m_mcGraph.clear();
  m_generator.generateEvent(m_mcGraph);
  m_mcGraph.generateList("", MCParticleGraph::c_setDecayInfo | MCParticleGraph::c_checkCyclic);

//   if (m_fileExtraInfo) {
//     m_th1dSDif->Fill(m_generator.getSDif(), 1.);
//   }
}

//-----------------------------------------------------------------
//                 Beam energy
//-----------------------------------------------------------------
double PhokharaInputModule::getBeamEnergyCM(double E1,
                                            double E2,
                                            double crossing_angle)
{

  double m = Const::electronMass;
  double ca = cos(crossing_angle);

  double P1 = sqrt(E1 * E1 - m * m);
  double P2 = sqrt(E2 * E2 - m * m);
  Double_t Etotcm = sqrt(2.*m * m + 2.*(E1 * E2 + P1 * P2 * ca));
  return Etotcm;
}

//-----------------------------------------------------------------
//                 Terminate
//-----------------------------------------------------------------
void PhokharaInputModule::terminate()
{

  m_generator.term();
}
