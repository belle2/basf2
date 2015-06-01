/************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014  Belle II Collaboration                              *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Torben Ferber                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <generators/modules/teegginput/TeeggInputModule.h>
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
REG_MODULE(TeeggInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TeeggInputModule::TeeggInputModule() : Module()
{
  //Set module properties
  setDescription("Generates events with TEEGG.");

  //Parameter definition
  addParam("VacuumPolarization", m_vacPol, "Vacuum polarization: off, hadr5 (Jegerlehner, default) or hmnt (Teubner)",
           std::string("OFF"));
  addParam("CMSEnergy", m_cmsEnergy, "CMS energy [GeV] (default: take from xml)", 0.0);
  addParam("BoostMode", m_boostMode, "The mode of the boost (0 = no boost, 1 = Belle II, 2 = Belle)", 0);
  addParam("ExtraFile", m_fileNameExtraInfo, "ROOT file that contains additional information.", std::string(""));
  addParam("TEVETO", m_TEVETO, "Maximum theta of e+ in final state (in deg)", 0.1);
  addParam("TEMIN", m_TEMIN, "Minimum angle between the e- and -z axis (EGAMMA conf. only) (in deg)", 20.0);
  addParam("TGMIN", m_TGMIN, "Minimum angle between the gamma and -z axis (in deg)",  20.0);
  addParam("TGVETO", m_TGVETO, "Maximum angle between the gamma and -z axis(ETRON conf. only) (in deg)", 0.05);
  addParam("EEMIN", m_EEMIN, "Minimum energy of the e- (EGAMMA and ETRON conf.) (in GeV)", 2.0);
  addParam("EGMIN", m_EGMIN, "Minimum energy of the gamma (EGAMMA and GAMMA conf.) (in GeV)", 2.0);
  addParam("PEGMIN", m_PEGMIN, "Minimum phi sep of e-gamma (EGAMMA config with hard rad. corr.) (in deg)", 0.0);
  addParam("EEVETO", m_EEVETO, "Minimum energy to veto (GAMMA config with hard rad. corr.) (in GeV)", 0.0);
  addParam("EGVETO", m_EGVETO, "Minimum energy to veto (ETRON/GAMMA config with hard rad. corr.) (in GeV)", 0.0);
  addParam("PHVETO", m_PHVETO, "Minimum phi sep to veto (ETRON/GAMMA config with hard rad. corr. (in deg)", 0.0);
  addParam("CUTOFF", m_CUTOFF, "Cutoff energy for radiative corrections (in CM frame) (in GeV)", 0.250);
  addParam("EPS", m_EPS, "Param. epsilon_s (smaller val. increases sampling of k_s^pbc)", 0.01);
  addParam("FRAPHI", m_FRAPHI, "Fraction of time phi_ks is generated with peak(hard rad corr)", 0.0);
  addParam("EPSPHI", m_EPSPHI, "Param. epsilon_phi ('cutoff' of the phi_ks peak)", 1.0e-4);
  addParam("WGHT1M", m_WGHT1M, "Maximum weight for generation of QP0, cos(theta QP)", 1.001);
  addParam("WGHTMX", m_WGHTMX, "Maximum weight for the trial events", 1.000);
  addParam("RADCOR", m_sRADCOR, "Specifies radiative correction (NONE, SOFT or HARD)", std::string("NONE"));
  addParam("CONFIG", m_sCONFIG, "Specifies the event configuration (EGAMMA, GAMMA, GAMMAE or ETRON)", std::string("EGAMMA"));
  addParam("MATRIX", m_sMATRIX, "Specifies which eeg matrix element (BK, BKM2, TCHAN or EPA)", std::string("BKM2"));
  addParam("MTRXGG", m_sMTRXGG, "Specifies which eegg matrix element (EPADC, BEEGG or MEEGG)", std::string("EPADC"));
  addParam("UNWGHT", m_UNWGHT, "If true then generate unweighted events", 1);
}


TeeggInputModule::~TeeggInputModule()
{

}


void TeeggInputModule::initialize()
{
  StoreArray<MCParticle>::registerPersistent();

  //Depending on the settings, use the Belle II or Belle boost
  double ecm = -1.; //center of mass energy, >0 if boost is set

  // initialize
  m_fileExtraInfo = NULL;
  m_ntuple = NULL;

  // open extrafile that will contain VP correction weights
  if (m_fileNameExtraInfo != "") {
    m_fileExtraInfo = new TFile(m_fileNameExtraInfo.c_str(), "RECREATE") ;
    m_fileExtraInfo->cd();
    m_ntuple = new TNtuple("ntuple", "data", "t:w2:weight:reansk:fullansk:reansk2:fullansk2");

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

  m_generator.setVacPol(m_vacPol);
  m_generator.setTEVETO(m_TEVETO);
  m_generator.setTEMIN(m_TEMIN);
  m_generator.setTGMIN(m_TGMIN);
  m_generator.setTGVETO(m_TGVETO);
  m_generator.setEEMIN(m_EEMIN);
  m_generator.setEGMIN(m_EGMIN);
  m_generator.setPEGMIN(m_PEGMIN);
  m_generator.setEEVETO(m_EEVETO);
  m_generator.setEGVETO(m_EGVETO);
  m_generator.setPHVETO(m_PHVETO);
  m_generator.setCUTOFF(m_CUTOFF);
  m_generator.setEPS(m_EPS);
  m_generator.setFRAPHI(m_FRAPHI);
  m_generator.setEPSPHI(m_EPSPHI);
  m_generator.setWGHT1M(m_WGHT1M);
  m_generator.setWGHTMX(m_WGHTMX);
  m_generator.setRADCOR(m_sRADCOR);
  m_generator.setCONFIG(m_sCONFIG);
  m_generator.setMATRIX(m_sMATRIX);
  m_generator.setMTRXGG(m_sMTRXGG);
  m_generator.setUNWGHT(m_UNWGHT);

  m_generator.init();

}


void TeeggInputModule::event()
{
  m_mcGraph.clear();
  m_generator.generateEvent(m_mcGraph);
  m_mcGraph.generateList("", MCParticleGraph::c_setDecayInfo | MCParticleGraph::c_checkCyclic);

  if (m_fileExtraInfo) {
    m_ntuple->Fill(m_generator.getT(), m_generator.getW2(), m_generator.getWeight(), m_generator.getReANSK(), m_generator.getFullANSK(),
                   m_generator.getReANSK2(), m_generator.getFullANSK2());
  }
}

double TeeggInputModule::getBeamEnergyCM(double E1,
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


void TeeggInputModule::terminate()
{
  if (m_fileExtraInfo) {
    m_fileExtraInfo->cd();
    m_ntuple->Write();
    delete m_fileExtraInfo;
  }

  m_generator.term();
}
