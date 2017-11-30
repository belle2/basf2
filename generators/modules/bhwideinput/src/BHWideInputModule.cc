/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Torben Ferber                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/bhwideinput/BHWideInputModule.h>

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

BHWideInputModule::BHWideInputModule() : Module(), m_initial(BeamParameters::c_smearVertex)
{
  //Set module properties
  setDescription("Generates radiative BhaBha scattering events with BHWide.");

  //Parameter definition
  addParam("ScatteringAngleRangePositron", m_ScatteringAngleRangePositron,
           "Min [0] and Max [1] value for the scattering angle [deg] of the positron.", make_vector(15.0, 165.0));
  addParam("ScatteringAngleRangeElectron", m_ScatteringAngleRangeElectron,
           "Min [0] and Max [1] value for the scattering angle [deg] of the electron.", make_vector(15.0, 165.0));
  addParam("MaxAcollinearity", m_maxAcollinearity, "Maximum acollinearity angle between finale state leptons/photons [degree]",
           180.0);
  addParam("MinEnergy", m_eMin, "Minimum energy for electrons in the final state [GeV] (default: 0.2 GeV)", 0.2);
  addParam("VacuumPolarization", m_vacPolString,
           "Vacuum polarization: off (off - EW off, too), Burkhardt89 (bhlumi), Eidelman/Jegerlehner95 (eidelman) or Burkhardt/Pietrzyk95 (burkhardt)",
           std::string("burkhardt"));
  addParam("WtMax", m_wtMax, "Maximum of weight (wtmax, default: 3.0), if <0: internal maximum search", 3.);
  addParam("WeakCorrections", m_weakCorrections, "EW correction ON/OFF", true);

  //initialize member variables
  m_vacPol = BHWide::PhotonVacPolarization::PP_BURKHARDT;

}


BHWideInputModule::~BHWideInputModule()
{

}


void BHWideInputModule::initialize()
{
  StoreArray<MCParticle> mcparticle;
  mcparticle.registerInDataStore();

  //Beam Parameters, initial particle - BHWIDE cannot handle beam energy spread
  m_initial.initialize();

}


void BHWideInputModule::event()
{
  // Check if the BeamParameters have changed (if they do, abort the job! otherwise cross section calculation will be a nightmare.)
  if (m_beamParams.hasChanged()) {
    if (!m_initialized) {
      initializeGenerator();
    } else {
      B2FATAL("BHWideInputModule::event(): BeamParameters have changed within a job, this is not supported for BHWide!");
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



void BHWideInputModule::terminate()
{
  m_generator.term();

  B2RESULT("BHWideInputModule: Total cross section: " << m_generator.getCrossSection() * 0.001 << " nb +- " <<
           m_generator.getCrossSection() *
           m_generator.getCrossSectionError() * 0.001 << " nb");
}

void BHWideInputModule::initializeGenerator()
{
  const BeamParameters& nominal = m_initial.getBeamParameters();
  double ecm = nominal.getMass();

  //   m_generator.enableBoost(m_boostMode > 0);
  m_generator.setScatAnglePositron(vectorToPair(m_ScatteringAngleRangePositron, "ScatteringAngleRangePositron"));
  m_generator.setScatAngleElectron(vectorToPair(m_ScatteringAngleRangeElectron, "ScatteringAngleRangeElectron"));

  m_generator.setMinEnergyFinalStatePos(m_eMin);
  m_generator.setMinEnergyFinalStateElc(m_eMin);
  m_generator.setMaxAcollinearity(m_maxAcollinearity);
  m_generator.setMaxRejectionWeight(m_wtMax);

  if (m_weakCorrections == 0) {
    m_generator.enableWeakCorrections(0);
  } else {
    m_generator.enableWeakCorrections(1);
  }

  //vacuum polarization (BHWide::PhotonVacPolarization)
  if (m_vacPolString == "off") {
    m_vacPol = BHWide::PhotonVacPolarization::PP_OFF;
    //need to switch off weak correction, otherwise BHWide will abort
    if (m_weakCorrections == 1) {
      B2INFO("BHWideInputModule: Switching OFF EW corrections");
    }
    m_generator.enableWeakCorrections(0);
  } else if (m_vacPolString == "bhlumi") m_vacPol = BHWide::PhotonVacPolarization::PP_BHLUMI;
  else if (m_vacPolString == "burkhardt") m_vacPol = BHWide::PhotonVacPolarization::PP_BURKHARDT;
  else if (m_vacPolString == "eidelman") m_vacPol = BHWide::PhotonVacPolarization::PP_EIDELMAN;
  else B2FATAL("BHWideInputModule: Vacuum Polarization option does not exist: " << m_vacPolString);
  m_generator.setPhotonVacPolarization(m_vacPol);

  m_generator.setCMSEnergy(ecm);

  m_generator.init();

  m_initialized = true;

}
