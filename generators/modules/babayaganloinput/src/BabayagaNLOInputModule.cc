/************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014  Belle II Collaboration                              *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Torben Ferber                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <generators/modules/babayaganloinput/BabayagaNLOInputModule.h>
#include <generators/utilities/cm2LabBoost.h>

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

BabayagaNLOInputModule::BabayagaNLOInputModule() : Module()
{
  //Set module properties
  setDescription("Generates radiative Bhabha scattering and exclusive two-photon events with Babayaga.NLO.");

  //Parameter definition
  addParam("VacuumPolarization", m_vacPol, "Vacuum polarization: off, hadr5 (Jegerlehner, default) or hmnt (Teubner)",
           std::string("hadr5"));
  addParam("Model", m_model, "Model: exp (default) or ps", std::string("exp"));
  addParam("FinalState", m_finalState, "Final state: ee (default), mm (not recommended) or gg", std::string("ee"));
  addParam("MinEnergyFrac", m_eMinFrac,
           "Fractional minimum energy for leptons (ee mode) or photons (gg mode) in the final state [fraction of ECMS]", -1.0);
  addParam("MinEnergy", m_eMin, "Minimum energy for leptons (ee mode) or photons (gg mode) in the final state [GeV]", 0.150);
  addParam("Epsilon", m_epsilon, "Soft/hard photon separator [fraction of ECMS/2]", 5.e-4);
  addParam("MaxAcollinearity", m_maxAcollinearity, "Maximum acollinearity angle between finale state leptons/photons [degree]",
           180.0);
  addParam("CMSEnergy", m_cmsEnergy, "CMS energy [GeV] (default: take from xml)", 0.0);
  addParam("FMax", m_fMax, "Maximum of differential cross section weight (fmax)", 50000.);
  addParam("SearchMax", m_nSearchMax, "Number of events used to search for maximum of differential cross section", 500000);
  addParam("BoostMode", m_boostMode, "The mode of the boost (0 = no boost, 1 = Belle II, 2 = Belle)", 1);
  addParam("ScatteringAngleRange", m_ScatteringAngleRange, "Min [0] and Max [1] value for the scattering angle [deg].",
           make_vector(15.0, 165.0));
  addParam("ExtraFile", m_fileNameExtraInfo, "ROOT file that contains additional information.", std::string(""));

  //initialize member variables
  m_fileExtraInfo = 0;
  m_th1dSDif = 0;
}


BabayagaNLOInputModule::~BabayagaNLOInputModule()
{

}


void BabayagaNLOInputModule::initialize()
{
  StoreArray<MCParticle>::registerPersistent();

  //Depending on the settings, use the Belle II or Belle boost
  double ecm = -1.; //center of mass energy, >0 if boost is set

  //open extrafile
  if (m_fileNameExtraInfo != "") {
    m_fileExtraInfo = new TFile(m_fileNameExtraInfo.c_str(), "RECREATE") ;
    m_fileExtraInfo->cd();
    m_th1dSDif = new TH1D("sdif", "sdif", 1000, 0., 100000.);
  }

  //Depending on the settings, use the Belle II or Belle boost
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
      ecm = getBeamEnergyCM(electronBeamEnergy, positronBeamEnergy, crossingAngle * 0.001);
    }
  }

  m_generator.enableBoost(m_boostMode > 0);

  //overwrite user setting if boost is enabled!
  if (m_boostMode) {
    if (m_cmsEnergy > 0.) { //user has set a cms energy... should not be
      B2WARNING("CM energy set manually, but boost mode enabled, resetting ECM to " << ecm);
    }
    m_generator.setCMSEnergy(ecm);

  } else {
    if (m_cmsEnergy <= 0. && ecm < 0.) {
      B2FATAL("CM energy not set: " << m_cmsEnergy);
    }
    m_generator.setCMSEnergy(m_cmsEnergy);
  }

  m_generator.setScatAngle(vectorToPair(m_ScatteringAngleRange, "ScatteringAngleRange"));
//   m_generator.setCMSEnergy(m_cmsEnergy);
  m_generator.setMaxAcollinearity(m_maxAcollinearity);
  m_generator.setFinalState(m_finalState);
  m_generator.setEpsilon(m_epsilon);
  m_generator.setVacPol(m_vacPol);
  m_generator.setModel(m_model);

  if (m_eMinFrac >= 0.) {
    B2INFO("Setting EMIN using fraction " << m_eMinFrac << " of CMS energy: cut=" << m_eMinFrac * m_cmsEnergy << " GeV")
    m_generator.setMinEnergy(m_eMinFrac * m_cmsEnergy);
  } else m_generator.setMinEnergy(m_eMin);

  if (m_fMax >= 0.) {
    B2INFO("Setting FMAX (no maximum search performed) =" << m_fMax)
    m_generator.setFMax(m_fMax);
    m_generator.setNSearchMax(-1);

  } else m_generator.setNSearchMax(m_nSearchMax);

  m_generator.init();

}


void BabayagaNLOInputModule::event()
{
  m_mcGraph.clear();
  m_generator.generateEvent(m_mcGraph);
  m_mcGraph.generateList("", MCParticleGraph::c_setDecayInfo | MCParticleGraph::c_checkCyclic);

  if (m_fileExtraInfo) {
    m_th1dSDif->Fill(m_generator.getSDif(), 1.);
  }
}

double BabayagaNLOInputModule::getBeamEnergyCM(double E1,
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


void BabayagaNLOInputModule::terminate()
{
  if (m_fileExtraInfo) {
    m_fileExtraInfo->cd();
    m_th1dSDif->Write();
    delete m_fileExtraInfo;
  }

  m_generator.term();
}
