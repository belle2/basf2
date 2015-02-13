/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/bhwideinput/BHWideInputModule.h>
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
REG_MODULE(BHWideInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BHWideInputModule::BHWideInputModule() : Module()
{
  //Set module properties
  setDescription("Generates radiative BhaBha scattering events with BHWide.");

  //Parameter definition
  addParam("BoostMode", m_boostMode, "The mode of the boost (0 = no boost, 1 = Belle II, 2 = Belle)", 1);
  addParam("ScatteringAngleRangePositron", m_ScatteringAngleRangePositron, "Min [0] and Max [1] value for the scattering angle [deg] of the positron.", make_vector(15.0, 165.0));
  addParam("ScatteringAngleRangeElectron", m_ScatteringAngleRangeElectron, "Min [0] and Max [1] value for the scattering angle [deg] of the electron.", make_vector(15.0, 165.0));

  addParam("MaxAcollinearity", m_maxAcollinearity, "Maximum acollinearity angle between finale state leptons/photons [degree]", 180.0);
  addParam("CMSEnergy", m_cmsEnergy, "CMS energy [GeV] (default: take from xml)", 0.0);
  addParam("MinEnergy", m_eMin, "Minimum energy for electrons in the final state [GeV] (default: 0.2 GeV)", 0.2);
  addParam("VacuumPolarization", m_vacPolString, "Vacuum polarization: off (off), Burkhardt89 (bhlumi), Eidelman/Jegerlehner95 (eidelman) or Burkhardt/Pietrzyk95 (burkhardt)", std::string("burkhardt"));
  addParam("WtMax", m_wtMax, "Maximum of weight (wtmax, default: 3.0), if <0: internal maximum search", 3.);
}


BHWideInputModule::~BHWideInputModule()
{

}


void BHWideInputModule::initialize()
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
      TLorentzVector boostVector(pE * sin(crossingAngle * 0.001), 0., pE * cos(crossingAngle * 0.001) - pzP, electronBeamEnergy + positronBeamEnergy);
      m_generator.setBoost(boostVector.BoostVector());

      //get CMS energy
      std::cout << electronBeamEnergy << std::endl;
      std::cout << positronBeamEnergy << std::endl;
      std::cout << crossingAngle  << std::endl;
      ecm = getBeamEnergyCM(electronBeamEnergy, positronBeamEnergy, crossingAngle * 0.001);
      std::cout << ecm  << std::endl;

    }
  }

  m_generator.enableBoost(m_boostMode > 0);
  m_generator.setScatAnglePositron(vectorToPair(m_ScatteringAngleRangePositron, "ScatteringAngleRangePositron"));
  m_generator.setScatAngleElectron(vectorToPair(m_ScatteringAngleRangeElectron, "ScatteringAngleRangeElectron"));

  m_generator.setMinEnergyFinalStatePos(m_eMin);
  m_generator.setMinEnergyFinalStateElc(m_eMin);
  m_generator.setMaxAcollinearity(m_maxAcollinearity);
  m_generator.setMaxRejectionWeight(m_wtMax);

  //vacuum polarization (BHWide::PhotonVacPolarization)
  if (m_vacPolString == "off") {
    m_vacPol = BHWide::PhotonVacPolarization::PP_OFF;
    //need to switch off weak correction, otherwise BHWide will abort
    m_generator.enableWeakCorrections(0);
  } else if (m_vacPolString == "bhlumi") m_vacPol = BHWide::PhotonVacPolarization::PP_BHLUMI;
  else if (m_vacPolString == "burkhardt") m_vacPol = BHWide::PhotonVacPolarization::PP_BURKHARDT;
  else if (m_vacPolString == "eidelman") m_vacPol = BHWide::PhotonVacPolarization::PP_EIDELMAN;
  else B2FATAL("Vacuum Polarization option does not exist: " << m_vacPolString);
  m_generator.setPhotonVacPolarization(m_vacPol);

  //check combination of CMS energy and boost mode compatibility
  if (m_boostMode) {
    if (m_cmsEnergy > 0.) { //user has set a cms energy... should not be
      B2FATAL("CM energy set manually, but boost mode enabled");
    } else m_generator.setCMSEnergy(ecm);
  } else {
    if (m_cmsEnergy <= 0.) {
      B2FATAL("No boost enabled and CM energy not set: " << m_cmsEnergy);
    } else m_generator.setCMSEnergy(m_cmsEnergy);
  }

  m_generator.init();
}


void BHWideInputModule::event()
{
  m_mcGraph.clear();
  m_generator.generateEvent(m_mcGraph);
  m_mcGraph.generateList("", MCParticleGraph::c_setDecayInfo | MCParticleGraph::c_checkCyclic);
}


double BHWideInputModule::getBeamEnergyCM(double E1,
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


void BHWideInputModule::terminate()
{
  m_generator.term();

  B2INFO(">>> Total cross section: " << m_generator.getCrossSection() * 0.001 << " nb +- " << m_generator.getCrossSection() * m_generator.getCrossSectionError() * 0.001 << " nb")
}
