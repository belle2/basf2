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
#include <generators/dataobjects/MCParticleGraph.h>
#include <generators/utilities/cm2LabBoost.h>
#include <framework/core/Environment.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Unit.h>

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
  setDescription("EvtGen input");
  setPropertyFlags(c_Input);

  //Parameter definition
  addParam("userDECFile", m_userDECFileName, "user DECfile name", string(""));
  addParam("DECFile", m_DECFileName, "standard DECfile name should be provided: default file is in externals/share/evtgen/DECAY.DEC",
           Environment::Instance().getExternalsPath() + "/share/evtgen/DECAY.DEC");
  addParam("pdlFile", m_pdlFileName, "standard pdlfile name should be provided: default file is in externals/share/evtgen/evt.pdl",
           Environment::Instance().getExternalsPath() + "/share/evtgen/evt.pdl");
  addParam("ParentParticle", m_parentParticle, "Parent Particle Name", string("Upsilon(4S)"));
  addParam("boost2LAB", m_boost2LAB, "Boolean to indicate whether the particles should be boosted to LAB frame", true);
  addParam("HER_Energy", m_EHER, "Energy for HER[GeV]", 7 * Unit::GeV);
  addParam("LER_Energy", m_ELER, "Energy for LER[GeV]", 4 * Unit::GeV);
  addParam("HER_Spread", m_HER_Espread, "Energy spread for HER[GeV]", 0.00513 * Unit::GeV);
  addParam("LER_Spread", m_LER_Espread, "Energy spread for LER[GeV]", 0.002375 * Unit::GeV);
  addParam("CrossingAngle", m_crossing_angle, "Beam pipe crossing angle[mrad]", 83 * Unit::mrad);
  addParam("RotationAngle", m_angle, "Rotation with respect to e- beampie[mrad]", 41.5 * Unit::mrad);
}


void EvtGenInputModule::initialize()
{
  B2INFO("starting initialisation of EvtGen Input Module. ");

  //setup the DECAY files:
  if (!m_userDECFileName.empty())
    m_Ievtgen.setup(m_DECFileName, m_pdlFileName, m_parentParticle, m_userDECFileName);
  else
    m_Ievtgen.setup(m_DECFileName, m_pdlFileName, m_parentParticle);


  //Initialize MCParticle collection
  StoreArray<MCParticle>::registerPersistent();

  B2INFO("finished initialising the EvtGen Input Module. ");

}


void EvtGenInputModule::event()
{
  B2INFO("Starting event simulation.");


  //Initialize the beam energy for each event separatly

  double mEle = Unit::electronMass;

  double angleLerToB = M_PI - m_angle * Unit::mrad;
  double Eler = gRandom->Gaus(m_ELER, m_LER_Espread) * Unit::GeV; //Beam energy spread
  double pLerZ = Eler * cos(angleLerToB);
  double pLerX = Eler * sin(angleLerToB);
  double pLerY = 0.;


  double angleHerToB = (m_crossing_angle * Unit::mrad) - (m_angle * Unit::mrad);
  double Eher = gRandom->Gaus(m_EHER, m_HER_Espread) * Unit::GeV; //Beam energy spread
  double pHerZ = Eher * cos(angleHerToB);
  double pHerX = Eher * sin(angleHerToB);
  double pHerY = 0.;

  TLorentzVector vlLer;
  vlLer.SetXYZM(pLerX, pLerY, pLerZ, mEle);

  TLorentzVector vlHer;
  vlHer.SetXYZM(pHerX, pHerY, pHerZ, mEle);

  TLorentzVector pParentParticle;

  pParentParticle = (vlHer + vlLer);

  //end initialization

  //Do we need to boost?
  if (!m_boost2LAB) {
    double cross_angle = m_crossing_angle * Unit::mrad;
    double angle = m_angle * Unit::mrad;
    m_Ievtgen.m_labboost = getBoost(Eher, Eler, cross_angle, angle);
  }

  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  mpg.clear();
  int nPart =  m_Ievtgen.simulateEvent(mpg, pParentParticle);
  B2INFO("Simulated event " << eventMetaDataPtr->getEvent() << " with " << nPart << " particles.");

}

