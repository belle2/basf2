/************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015  Belle II Collaboration                              *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Torben Ferber                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <generators/modules/babayaganloinput/BabayagaNLOInputModule.h>

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
REG_MODULE(BabayagaNLOInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

extern "C" {
  double babayaganlo_getrandomcmsenergy_()
  {
    return BabayagaNLOInputModule::generateCMSEnergy();
  }
}

// BabayagaNLOInputModule::BabayagaNLOInputModule() : Module(), s_initial(BeamParameters::c_smearALL)
InitialParticleGeneration BabayagaNLOInputModule::s_initial{BeamParameters::c_smearALL};

BabayagaNLOInputModule::BabayagaNLOInputModule() : Module()
{
  //Set module properties
  setDescription("Generates radiative Bhabha scattering and exclusive two-photon events with Babayaga.NLO.");

  //Parameter definition
  addParam("VacuumPolarization", m_vacPol, "Vacuum polarization: off, hadr5 (Jegerlehner) or hlmnt (Teubner, default)",
           std::string("hlmnt"));
  addParam("Model", m_model, "Model: ps or matched (default)", std::string("matched"));
  addParam("Order", m_order, "Order: born, alpha, exp (default)", std::string("exp"));
  addParam("Mode", m_mode, "Mode: weighted or unweighted (default)", std::string("unweighted"));
  addParam("FinalState", m_finalState, "Final state: ee (default), mm or gg", std::string("ee"));
  addParam("MinEnergyFrac", m_eMinFrac,
           "Fractional minimum energy for leptons (ee or mm mode) or photons (gg mode) in the final state [fraction of ECMS]", -1.0);
  addParam("MinEnergy", m_eMin, "Minimum energy for leptons (ee or mm mode) or photons (gg mode) in the final state [GeV]", 0.10);
  addParam("Epsilon", m_epsilon, "Soft/hard photon separator [fraction of ECMS/2], must be <=1e-7 for ee and mumu, <=1e-5 for gg",
           1.e-7);
  addParam("MaxAcollinearity", m_maxAcollinearity, "Maximum acollinearity angle between finale state leptons/photons [degree]",
           180.0);
  addParam("FMax", m_fMax, "Maximum of differential cross section weight (fmax)", -1.);
  addParam("NPhotons", m_nPhot, "Fixed number of (hard) photons are generated, -1 for any number", -1);
  addParam("SearchMax", m_nSearchMax, "Number of events used to search for maximum of differential cross section", 500000);
  addParam("ScatteringAngleRange", m_ScatteringAngleRange, "Min [0] and Max [1] value for the scattering angle [deg].",
           make_vector(15.0, 165.0));
  addParam("ExtraFile", m_fileNameExtraInfo, "ROOT file that contains additional information.", std::string(""));
  addParam("DebugEnergySpread", m_Spread, "TEMPORARY SOLUTION! Approximate energy spread per beam (CMS)", 5.e-3);
  addParam("VPUncertainty", m_Uncertainty, "Calculate VP uncertainty by internal reweighting", false);
  addParam("NSKDataFile", m_NSKDataFile, "File that contain VP data from Novosibirsk (nsk)",
           FileSystem::findFile("/data/generators/babayaganlo/vpol_novosibirsk.dat"));

  // user cuts
  addParam("UserMode", m_userMode, "User mode similar to TEEGG: ETRON, EGAMMA, GAMMA or PRESCALE or NONE (default)",
           std::string("NONE"));
  addParam("EEMIN", m_eemin, "Minimum CMS energy of the tagged e-/e+ (GeV)", -1.0);
  addParam("TEMIN", m_temin, "Minimum CMS angle between the tagged e-/e+ and -z axis (deg)", -1.0);
  addParam("EGMIN", m_egmin, "Minimum CMS energy of the gamma (GeV)", -1.0);
  addParam("TGMIN", m_tgmin, "Minimum CMS angle between the gamma and -z axis (deg)",  -1.0);
  addParam("EEVETO", m_eeveto, "Minimum CMS energy to veto e-/e+ (GeV)", -1.0);
  addParam("TEVETO", m_teveto, "Maximum CMS theta of e-/e+ in final state (deg)", -1.0);
  addParam("EGVETO", m_egveto, "Minimum CMS energy to veto gamma (GeV)", -1.0);
  addParam("TGVETO", m_tgveto, "Maximum CMS angle between the gamma and -z axis (deg)", -1.0);
  addParam("MaxPrescale", m_maxprescale, "Maximum prescale factor (used for maximum differential cross section)", 1.0);

  //initialize member variables
  m_fileExtraInfo = 0;
  m_th1dSDif = 0;
}

BabayagaNLOInputModule::~BabayagaNLOInputModule()
{

}

void BabayagaNLOInputModule::initialize()
{
  //Initialize MCParticle collection
  StoreArray<MCParticle> mcparticle;
  mcparticle.registerInDataStore();

  //open extrafile
  if (m_fileNameExtraInfo != "") {
    m_fileExtraInfo = new TFile(m_fileNameExtraInfo.c_str(), "RECREATE") ;
    m_fileExtraInfo->cd();
    m_th1dSDif = new TH1D("sdif", "sdif", 1000, 0., 100000.);
  }

  //Initialize initial particle for beam parameters.
  s_initial.initialize();

  // Initialize ExtraInfo (hold prescale values)
  m_generator.initExtraInfo();


}

void BabayagaNLOInputModule::event()
{

  // Check if the BeamParameters have changed (if they do, abort the job! otherwise cross section calculation will be a nightmare.)
  if (m_beamParams.hasChanged()) {
    if (!m_initialized) {
      initializeGenerator();
    } else {
      B2FATAL("BabayagaNLOInputModule::event(): BeamParameters have changed within a job, this is not supported for BABAYAGA!");
    }
  }

  // initial particle from beam parameters
  MCInitialParticles& initial = s_initial.generate();

  // CM energy
  double ecm = initial.getMass();

  // true boost (per event!)
  TLorentzRotation boost = initial.getCMSToLab();

  // vertex
  TVector3 vertex = initial.getVertex();

  m_mcGraph.clear();
  m_generator.generateEvent(m_mcGraph, ecm, vertex, boost); // actual generator call

  m_mcGraph.generateList("", MCParticleGraph::c_setDecayInfo | MCParticleGraph::c_checkCyclic);

}

void BabayagaNLOInputModule::terminate()
{

  m_generator.term();
}

void BabayagaNLOInputModule::initializeGenerator()
{

  // generator parameters
  m_generator.setScatAngle(vectorToPair(m_ScatteringAngleRange, "ScatteringAngleRange"));
  m_generator.setMaxAcollinearity(m_maxAcollinearity);
  m_generator.setFinalState(m_finalState);
  m_generator.setEpsilon(m_epsilon);
  m_generator.setVacPol(m_vacPol);
  m_generator.setOrder(m_order);
  m_generator.setModel(m_model);
  m_generator.setMode(m_mode);
  m_generator.setEnergySpread(m_Spread);
  m_generator.setVPUncertainty(m_Uncertainty);
  m_generator.setNPhotons(m_nPhot);
  m_generator.setNSKDataFile(m_NSKDataFile);
  m_generator.setEEMIN(m_eemin);
  m_generator.setTEMIN(m_temin);
  m_generator.setEGMIN(m_egmin);
  m_generator.setTGMIN(m_tgmin);
  m_generator.setEEVETO(m_eeveto);
  m_generator.setTEVETO(m_teveto);
  m_generator.setEGVETO(m_egveto);
  m_generator.setTGVETO(m_tgveto);
  m_generator.setUserMode(m_userMode);
  m_generator.setMaxPrescale(m_maxprescale);

  // set a nominal value for some intialization inside the generator
  const BeamParameters& nominal = s_initial.getBeamParameters();
  double ecmnominal = nominal.getMass();
  m_generator.setCmsEnergyNominal(ecmnominal);

  // check which of the emin values are used:
  // its either a fixed value or a fractional one
  // if both are provided, the fractional has priority
  if (m_eMinFrac >= 0.) {
    B2INFO("Setting EMIN using fraction " << m_eMinFrac << " of CMS energy: cut=" << m_eMinFrac * ecmnominal << " GeV");
    m_generator.setMinEnergy(m_eMinFrac * ecmnominal);
  } else m_generator.setMinEnergy(m_eMin);

  // check if a maximum weight is provided
  if (m_fMax > 0. && (m_mode == "unweighted" || m_mode == "uw")) {
    B2INFO("Setting FMAX manually (no maximum search performed) =" << m_fMax);
    m_generator.setFMax(m_fMax);
    m_generator.setNSearchMax(-1);
  } else if (m_mode == "weighted" || m_mode == "w") {
    m_generator.setNSearchMax(-1);
  } else {
    m_generator.setNSearchMax(m_nSearchMax);
  }

  m_generator.init();

  m_initialized = true;

}
