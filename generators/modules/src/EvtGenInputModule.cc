/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Susanne Koblitz                                          *
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
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/gearbox/GearDir.h>

#include <boost/format.hpp>
#include <boost/foreach.hpp>

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
}


void EvtGenInputModule::initialize()
{
  B2INFO("starting initialisation of EvtGen Input Module. ");

  //needs to be available to initialize things in beginRun()!
  StoreObjPtr<EventMetaData>::required();

  //setup the DECAY files:
  if (!m_userDECFileName.empty())
    m_Ievtgen.setup(m_DECFileName, m_pdlFileName, m_parentParticle, m_userDECFileName);
  else
    m_Ievtgen.setup(m_DECFileName, m_pdlFileName, m_parentParticle);


  //Initialize MCParticle collection
  StoreArray<MCParticle>::registerPersistent();

  if (m_inclusiveType == 0) m_inclusiveParticle = " ";
  if (m_inclusiveType != 0 && EvtPDL::getId(m_inclusiveParticle).getId() == -1) B2ERROR("User Specified Inclusive Particle does not exist !!") ;

  B2INFO("finished initialising the EvtGen Input Module. ");
}


void EvtGenInputModule::beginRun()
{
  GearDir ler("/Detector/SuperKEKB/LER/");
  GearDir her("/Detector/SuperKEKB/HER/");
  m_ELER = ler.getDouble("energy");
  m_EHER = her.getDouble("energy");
  m_LER_Espread = ler.getDouble("energyError");
  m_HER_Espread = her.getDouble("energyError");
  m_crossing_angle = her.getDouble("angle") - ler.getDouble("angle");
  m_angle = her.getDouble("angle");
}


void EvtGenInputModule::event()
{
  B2INFO("Starting event simulation.");

  double mEle;
  double angleLerToB, Eler, pLerX, pLerY, pLerZ;
  double angleHerToB, Eher, pHerX, pHerY, pHerZ;
  TLorentzVector vlLer, vlHer, pParentParticle;

  //Initialize the beam energy for each event separatly
  for (;;) {
    mEle = Const::electronMass;

    angleLerToB = M_PI - m_angle;
    Eler = gRandom->Gaus(m_ELER, m_LER_Espread); //Beam energy spread
    pLerZ = Eler * cos(angleLerToB);
    pLerX = Eler * sin(angleLerToB);
    pLerY = 0.;


    angleHerToB = m_crossing_angle - m_angle;
    Eher = gRandom->Gaus(m_EHER, m_HER_Espread); //Beam energy spread
    pHerZ = Eher * cos(angleHerToB);
    pHerX = Eher * sin(angleHerToB);
    pHerY = 0.;

    vlLer.SetXYZM(pLerX, pLerY, pLerZ, mEle);

    vlHer.SetXYZM(pHerX, pHerY, pHerZ, mEle);

    pParentParticle = (vlHer + vlLer);


    EvtId temp = EvtPDL::getId(m_parentParticle);

    if (temp.getId() == 93) temp = EvtPDL::getId("Upsilon(4S)");
    if (pParentParticle.M() > EvtPDL::getMinMass(temp) &&
        pParentParticle.M() < EvtPDL::getMaxMass(temp)) break;

  }

  //end initialization

  //Do we need to boost?
  if (!m_boost2LAB) {
    double cross_angle = m_crossing_angle;
    double angle = m_angle;
    m_Ievtgen.m_labboost = getBoost(Eher, Eler, cross_angle, angle);
  }

  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  mpg.clear();

  int nPart =  m_Ievtgen.simulateEvent(mpg, pParentParticle,  m_inclusiveType, m_inclusiveParticle);
  B2INFO("Simulated event " << eventMetaDataPtr->getEvent() << " with " << nPart << " particles.");

}

