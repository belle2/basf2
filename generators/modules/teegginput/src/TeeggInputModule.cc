/************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015  Belle II Collaboration                              *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Torben Ferber                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <generators/modules/teegginput/TeeggInputModule.h>

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

TeeggInputModule::TeeggInputModule() : Module(), m_initial(BeamParameters::c_smearVertex)
{
  //Set module properties
  setDescription("Generates events with TEEGG.");

  //Parameter definition
  addParam("VACPOL", m_sVACPOL, "Vacuum polarization: off, nsk (Novosibirsk) or hlmnt (Teubner et al., default)",
           std::string("hlmnt"));
  addParam("CMSEnergy", m_cmsEnergy, "CMS energy [GeV] (default: take from xml)", 0.0);
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
  addParam("UNWEIGHTED", m_UNWGHT, "If true then generate unweighted events", 1);
}

TeeggInputModule::~TeeggInputModule()
{

}


void TeeggInputModule::initialize()
{
  //Initialize MCParticle collection.
  StoreArray<MCParticle> mcparticle;
  mcparticle.registerInDataStore();

  //Initialize initial particle for beam parameters.
  m_initial.initialize();

  // Initialize ExtraInfo (holds vaccum polarization corrections)
  m_generator.initExtraInfo();

}


void TeeggInputModule::event()
{

  // Check if the BeamParameters have changed (if they do, abort the job! otherwise cross section calculation will be a nightmare.)
  if (m_beamParams.hasChanged()) {
    if (!m_initialized) {
      initializeGenerator();
    } else {
      B2FATAL("TeeggInputModule::event(): BeamParameters have changed within a job, this is not supported for TEEGG!");
    }
  }

  m_mcGraph.clear();

  // initial particle from beam parameters
  MCInitialParticles& initial = m_initial.generate();

  // true boost
  TLorentzRotation boost = initial.getCMSToLab();

  // vertex
  TVector3 vertex = initial.getVertex();

  m_generator.generateEvent(m_mcGraph, vertex, boost);
  m_mcGraph.generateList("", MCParticleGraph::c_setDecayInfo | MCParticleGraph::c_checkCyclic);
}


void TeeggInputModule::terminate()
{
  m_generator.term();
}

void TeeggInputModule::initializeGenerator()
{
  const BeamParameters& nominal = m_initial.getBeamParameters();
  m_cmsEnergy = nominal.getMass();

  m_generator.setCMSEnergy(m_cmsEnergy);
  m_generator.setVACPOL(m_sVACPOL);
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

  m_initialized = true;

}

