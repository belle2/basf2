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
  addParam("FinalState", m_finalState, "Final state: ee, mm or gg", std::string("ee"));
  addParam("MinEnergyFrac", m_eMinFrac, "Fractional minimum energy for leptons/photons in the final state [fraction of ECMS]", -1.0);
  addParam("MinEnergy", m_eMin, "Minimum energy for leptons/photons in the final state [GeV]", 0.150);
  addParam("Epsilon", m_epsilon, "Soft/hard photon separator [fraction of ECMS/2]", 5.e-4);
  addParam("MaxAcollinearity", m_maxAcollinearity, "Mximum acollinearity angle between finale state leptons/photons [degree]", 180.0);
  addParam("CMSEnergy", m_cmsEnergy, "CMS energy", 10.580);
  addParam("FMax", m_fMax, "Maximum of differential cross section weight (fmax)", -1.);
  addParam("SearchMax", m_nSearchMax, "Number of events used to search for maximum of differential cross section", 500000);
  addParam("BoostMode", m_boostMode, "The mode of the boost (0 = no boost, 1 = Belle II, 2 = Belle)", 0);
  addParam("ScatteringAngleRange", m_ScatteringAngleRange, "Min [0] and Max [1] value for the scattering angle [deg].", make_vector(15.0, 165.0));
  addParam("ExtraFile", m_fileNameExtraInfo, "ROOT file that contains additional information.", std::string(""));

}


BabayagaNLOInputModule::~BabayagaNLOInputModule()
{

}


void BabayagaNLOInputModule::initialize()
{
  StoreArray<MCParticle>::registerPersistent();

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
  } else {
    if (m_boostMode == 2) {

      //electron and positron beam energies (magic numbers from Jeremy)
      double electronBeamEnergy = 7.998213; // [GeV]
      double positronBeamEnergy = 3.499218; // [GeV]
      double crossingAngle = 22.0; //[mrad]

      double pzP = sqrt(positronBeamEnergy * positronBeamEnergy - 0.000510998918 * 0.000510998918);
      double pE  = sqrt(electronBeamEnergy * electronBeamEnergy - 0.000510998918 * 0.000510998918);
      TLorentzVector boostVector(pE * sin(crossingAngle * 0.001), 0., pE * cos(crossingAngle * 0.001) - pzP, electronBeamEnergy + positronBeamEnergy);
      m_generator.setBoost(boostVector.BoostVector());
    }
  }

  m_generator.enableBoost(m_boostMode > 0);
  m_generator.setScatAngle(vectorToPair(m_ScatteringAngleRange, "ScatteringAngleRange"));
  m_generator.setCMSEnergy(m_cmsEnergy);
  m_generator.setMaxAcollinearity(m_maxAcollinearity);
  m_generator.setFinalState(m_finalState);
  m_generator.setEpsilon(m_epsilon);

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


void BabayagaNLOInputModule::terminate()
{
  if (m_fileExtraInfo) {
    m_fileExtraInfo->cd();
    m_th1dSDif->Write();
    delete m_fileExtraInfo;
  }

  m_generator.term();
}
