/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Susanne Koblitz, Martin Ritter                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/EvtGenInputModule.h>
#include <generators/evtgen/EvtGenInterface.h>
#include <mdst/dataobjects/MCParticleGraph.h>
#include <generators/utilities/cm2LabBoost.h>
#include <framework/core/Environment.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/gearbox/GearDir.h>

#include <TRandom.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EvtGenInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EvtGenInputModule::EvtGenInputModule() : Module()
{
  //Set module properties
  //setDescription("EvtGen input");
  setDescription("EvtGenInput module. The module is served as an interface for EvtGen Event Generator so that the EvtGen generator can store the generated particles into MCParticles. The users need to provide their own decay mode based on the standard DECAY.DEC.");
  setPropertyFlags(c_Input);

  //Parameter definition
  addParam("userDECFile", m_userDECFileName, "user DECfile name", string(""));
  addParam("DECFile", m_DECFileName, "standard DECfile name should be provided: default file is in externals/share/evtgen/DECAY.DEC",
           Environment::Instance().getExternalsPath() + "/share/evtgen/DECAY.DEC");
  addParam("pdlFile", m_pdlFileName, "standard pdlfile name should be provided: default file is in externals/share/evtgen/evt.pdl",
           Environment::Instance().getExternalsPath() + "/share/evtgen/evt.pdl");
  addParam("ParentParticle", m_parentParticle, "Parent Particle Name", string("Upsilon(4S)"));
  addParam("InclusiveType", m_inclusiveType, "inclusive decay type (0: generic, 1: inclusive, 2: inclusive (charge conjugate)", 0);
  addParam("InclusiveParticle", m_inclusiveParticle, "Inclusive Particle Name", string(""));
  addParam("boost2LAB", m_boost2LAB, "Boolean to indicate whether the particles should be boosted to LAB frame", true);
  addParam("maxTries", m_maxTries, "Number of tries to generate a parent "
           "particle from the beam energies which fits inside the mass window "
           "before giving up", 100000);
}


void EvtGenInputModule::initialize()
{
  B2DEBUG(10, "starting initialisation of EvtGen Input Module. ");

  //setup the DECAY files:
  m_Ievtgen.setup(m_DECFileName, m_pdlFileName, m_parentParticle, m_userDECFileName);

  //Initialize MCParticle collection
  StoreArray<MCParticle>::registerPersistent();

  if (m_inclusiveType == 0) m_inclusiveParticle = "";
  if (m_inclusiveType != 0 && EvtPDL::getId(m_inclusiveParticle).getId() == -1) {
    B2ERROR("User Specified Inclusive Particle '" << m_inclusiveParticle
            << "' does not exist");
  }
  m_parentId = EvtPDL::getId(m_parentParticle);
  if (m_parentId.getId() == -1) {
    B2ERROR("User specified parent particle '" << m_parentParticle
            << "' does not exist");
  }

  B2DEBUG(10, "finished initialising the EvtGen Input Module.");
}


void EvtGenInputModule::beginRun()
{
  // Get beam information and save it to generate events with correct energy
  // and energy spread
  GearDir ler("/Detector/SuperKEKB/LER/");
  GearDir her("/Detector/SuperKEKB/HER/");
  m_ELER = ler.getEnergy("energy");
  m_EHER = her.getEnergy("energy");
  m_LER_Espread = ler.getEnergy("energyError");
  m_HER_Espread = her.getEnergy("energyError");
  m_angleLerToB = ler.getAngle("angle") + M_PI;
  m_angleHerToB = her.getAngle("angle");
  // These two are redundant but needed for current boost function. Remove once
  // boost function is fixed to take the same parameters as stored in DB
  m_crossing_angle = her.getAngle("angle") - ler.getAngle("angle");
  m_angle = her.getAngle("angle");
}

TLorentzVector EvtGenInputModule::createBeamParticle(double minMass, double maxMass)
{
  // so we need to generate the collision 4-momentum including beamspread
  double eLER(0), eHER(0);
  TLorentzVector vHER, vLER;
  // calculate sine/cosine here since they do not change
  const double cHER = cos(m_angleHerToB);
  const double sHER = sin(m_angleHerToB);
  const double cLER = cos(m_angleLerToB);
  const double sLER = sin(m_angleLerToB);
  // we try to generate the 4 momentum a maximum amount of times before we give
  // up
  for (int i = 0; i < m_maxTries; ++i) {
    // assume gaussian energy spread
    eHER = gRandom->Gaus(m_EHER, m_HER_Espread);
    eLER = gRandom->Gaus(m_ELER, m_LER_Espread);
    // set 4-vectors for the beams
    vHER.SetXYZM(eHER * sHER, 0., eHER * cHER, Const::electronMass);
    vLER.SetXYZM(eLER * sLER, 0., eLER * cLER, Const::electronMass);
    // calculate collision 4-vector
    TLorentzVector beam = (vHER + vLER);
    // check if we fullfill the mass window
    if (beam.M() >= minMass && beam.M() < maxMass) {
      // if we don't boost2Lab we need to boost to CMS because we are already
      // in lab. So get the boost and apply it
      if (!m_boost2LAB) {
        TLorentzRotation boost = getBoost(eHER, eLER, m_crossing_angle, m_angle).Inverse();
        beam = boost * beam;
      }
      // return the 4-vector
      return beam;
    }
  }
  //Apparently the beam energies don't match the particle mass we want to generate
  B2FATAL("Could not create parent particle within mass window: "
          << "HER=" << m_EHER << "+-" << m_HER_Espread  << " GeV, "
          << "LER=" << m_ELER << "+-" << m_LER_Espread  << " GeV, "
          << "minMass=" << minMass << " GeV, "
          << "maxMass=" << maxMass << " GeV");
  //This will never be reached so return empty to avoid warning
  return TLorentzVector(0, 0, 0, 0);
}


void EvtGenInputModule::event()
{
  B2DEBUG(10, "Starting event generation");
  TLorentzVector pParentParticle;
  //Initialize the beam energy for each event separatly
  if (m_parentId.getId() == 93) {
    //virtual photon, no mass window we accept everything
    pParentParticle = createBeamParticle();
  } else {
    //everything else needs to be in the mass window
    pParentParticle = createBeamParticle(EvtPDL::getMinMass(m_parentId),
                                         EvtPDL::getMaxMass(m_parentId));
  }
  //end initialization

  //clear existing MCParticles
  mpg.clear();
  //generate event.
  int nPart =  m_Ievtgen.simulateEvent(mpg, pParentParticle,  m_inclusiveType,
                                       m_inclusiveParticle);

  B2DEBUG(10, "EvtGen: generated event with " << nPart << " particles.");
}

