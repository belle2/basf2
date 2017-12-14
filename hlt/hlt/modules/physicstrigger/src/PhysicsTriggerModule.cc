/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua LI                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// own include
#include <hlt/hlt/modules/physicstrigger/PhysicsTriggerModule.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>

//framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>

// dataobjects
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <analysis/dataobjects/ParticleList.h>

//utilities
#include <analysis/VariableManager/Variables.h>
#include <analysis/VariableManager/PhysicsTriggerVariables.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <framework/utilities/FileSystem.h>

#include <boost/foreach.hpp>
#include <fstream>



using namespace Belle2;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PhysicsTrigger)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PhysicsTriggerModule::PhysicsTriggerModule() : Module()
{
  // Set module properties
  setDescription("skim physics processes");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("TriggerCategory", m_trgcategory, "the index of trigger category", 1);

  std::string emptyCut;
  addParam("PT", m_hltcut, "the selection criteria of physics trigger ", emptyCut);

}

PhysicsTriggerModule::~PhysicsTriggerModule()
{
}

void PhysicsTriggerModule::initialize()
{
  B2INFO("PhysicsTriggerModule processing");
  m_physicsTriggerInformation.registerInDataStore();
  m_cut = Variable::Cut::compile(m_hltcut);
}

void PhysicsTriggerModule::beginRun()
{
}

void PhysicsTriggerModule::event()
{

  setReturnValue(0);
  if (eventSelect()) setReturnValue(1);
}

void PhysicsTriggerModule::endRun()
{
}

void PhysicsTriggerModule::terminate()
{
}


bool  PhysicsTriggerModule::eventSelect()
{
  B2DEBUG(200, "User custom selection is open");
  bool Fval = false;

  bool fir = false;
  StoreArray<PhysicsTriggerInformation> phyTriInfos;
  if (!phyTriInfos.getEntries()) {
    fir = true;
    phyTriInfos.appendNew(PhysicsTriggerInformation());
  }
  PhysicsTriggerInformation& phyTriInfo = *phyTriInfos[0];
  if (fir) {
    eventInformation(phyTriInfo);
    B2DEBUG(200, "Fill PhysicsTrigger Infomation");
  }

  int bitsub = phyTriInfo.getnsubTrg(m_trgcategory - 1);
  int bittot = phyTriInfo.getntotTrg();
  const Particle* part = NULL;
  int totbitMask(0x0);
  int subbitMask(0x0);
  if (m_cut->check(part)) {
    subbitMask |= 0x1;
    subbitMask <<= bitsub;
    totbitMask |= 0x1;
    totbitMask <<= bittot;
    Fval = true;
  } else
    Fval = false;

  phyTriInfo.setPTSummary(totbitMask);
  phyTriInfo.setsubTrgResult(m_trgcategory - 1, subbitMask);
  phyTriInfo.setnsubTrg(m_trgcategory - 1);
  phyTriInfo.setntotTrg(1);
  return Fval;

}


void PhysicsTriggerModule::eventInformation(PhysicsTriggerInformation& phyTriInfo)
{

  Particle* p = NULL;
  int m_ngoodTracks = Variable::nTracksLE(p);
  int m_ngoodECLClusters = Variable::nClustersLE(p);
  int m_nKLMClusters = Variable::nKLMClustersLE(p);
  double m_Pzvis = Variable::VisiblePzLE(p);
  double m_Esum = Variable::EtotLE(p);
  double m_Evis = Variable::VisibleEnergyLE(p);
  double m_p1 = Variable::P1BhabhaLE(p);
  double m_p2 = Variable::P2BhabhaLE(p);
  double m_e1 = Variable::EC1LE(p);
  double m_e2 = Variable::EC2LE(p);
  double m_angleTT = Variable::maxAngleTTLE(p);
  double m_maxangleTT = Variable::maxAngleTTLE(p);
  double m_angleGG = Variable::AngleGGLE(p);

//The total ECL energy
  phyTriInfo.setNGoodTrack(m_ngoodTracks);
  phyTriInfo.setNGoodECLClusters(m_ngoodECLClusters);
  phyTriInfo.setNKLMClusters(m_nKLMClusters);
  phyTriInfo.setEvis(m_Evis);
  phyTriInfo.setPzvis(m_Pzvis);
  phyTriInfo.setEsum(m_Esum);
  phyTriInfo.setAngleTT(m_angleTT);
  phyTriInfo.setMaxAngleTT(m_maxangleTT);
  phyTriInfo.setAngleGG(m_angleGG);
  phyTriInfo.setP1(m_p1);
  phyTriInfo.setP2(m_p2);
  phyTriInfo.setE1(m_e1);
  phyTriInfo.setE2(m_e2);
}






