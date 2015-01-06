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
#include <skim/hlt/modules/physicstrigger/PhysicsTriggerModule.h>
#include <skim/hlt/dataobjects/PhysicsTriggerInformation.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>

//framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>

// dataobjects
#include "mdst/dataobjects/HLTTag.h"
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
  addParam("ParameterCutIndex", m_parameterCutIndex, "control selection criteria condition", 2);

  addParam("UserCustomOpen", m_userCustomOpen, "the switch of customing the selection criteria by user", 0);
  Variable::Cut::Parameter emptyCut;
  addParam("UserCustomCut", m_userCustomCut, "the user-custom selection criteria", emptyCut);
}

PhysicsTriggerModule::~PhysicsTriggerModule()
{
}

void PhysicsTriggerModule::initialize()
{
  B2INFO("PhysicsTriggerModule processing");
  StoreArray<HLTTag>::registerPersistent();
  StoreArray<PhysicsTriggerInformation>::registerPersistent();
  ReadParameters();
  m_cut.init(m_userCustomCut);
}

void PhysicsTriggerModule::beginRun()
{
}

void PhysicsTriggerModule::event()
{

  setReturnValue(0);
  initializeForEvent();
  eventInformation();
  if (m_userCustomOpen) {
    if (eventUserSelect()) setReturnValue(1);
  } else {
    eventSelect();
    if (m_summary)setReturnValue(2);
  }

}

void PhysicsTriggerModule::endRun()
{
}

void PhysicsTriggerModule::terminate()
{
}


void PhysicsTriggerModule::eventSelect()
{
  StoreArray<HLTTag> hltTags;
  if (!hltTags.getEntries()) {
    hltTags.appendNew(HLTTag());
    B2DEBUG(200, "No entry in hltTags");
  }
  HLTTag* hltTag = hltTags[0];
  m_summary = 0;
//hadronic
  int bitMask(0x1);
  bitMask <<= 2;
  if (m_ngoodTracks >= m_nTrackCutMin_had) {
    if (m_ngoodECLClusters >= m_nECLClusterCutMin_had) {
      if (m_Evis >= m_EvisCutMin_had) {
        if (m_Esum >= m_EsumCutMin_had && m_Esum <= m_EsumCutMax_had)
          m_summary |= bitMask;
        hltTag->Accept(HLTTag::Hadronic);
        B2DEBUG(200, "the event has passed hadronic selection criteria");
      }
    }
  }

//Tautau
  bitMask <<= 1;
  if (m_ngoodTracks >= m_nTrackCutMin_tau && m_ngoodTracks <= m_nTrackCutMax_tau) {
    if (m_Esum <= m_EsumCutMax_tau) {
      m_summary |= bitMask;
      hltTag->Accept(HLTTag::Tautau);
      B2DEBUG(200, "the event has passed tautau selection criteria");
    }
  }

//Gamma-pair
  bitMask <<= 1 ;
  if (m_ngoodTracks <= m_nTrackCutMax_digamma) {
    if (m_ngoodECLClusters >= m_nECLClusterCutMin_digamma) {
      if (m_e1 >= m_ECLClusterE1CutMin_digamma) {
        if (m_Esum >= m_EsumCutMin_digamma) {
          if (m_angleGG <= m_AngleGGCutMax_digamma) {
            m_summary |= bitMask;
            hltTag->Accept(HLTTag::LowMulti);
            B2DEBUG(200, "the event has passed LowMulti selection criteria");
          }
        }
      }
    }
  }

//Bhabha
  bitMask <<= 1 ;
  if (m_ngoodTracks >= m_nTrackCutMin_bhabha) {
    if (m_ngoodECLClusters >= m_nECLClusterCutMin_bhabha) {
      if (m_e1 >= m_ECLClusterE1CutMin_bhabha) {
        if (m_Esum >= m_EsumCutMin_bhabha) {
          if (m_angleTT <= m_AngleTTCutMax_bhabha) {
            m_summary |= bitMask;
            hltTag->Accept(HLTTag::Bhabha);
            B2DEBUG(200, "the event has passed Bhabha selection criteria");
          }
        }
      }
    }
  }


//Mu-pair
  bitMask <<= 1 ;
  if (m_ngoodTracks >= m_nTrackCutMin_dimu) {
    if (m_Esum >= m_EsumCutMin_dimu) {
      if (m_angleTT <= m_AngleTTCutMax_dimu) {
        m_summary |= bitMask;
        hltTag->Accept(HLTTag::Mumu);
        B2DEBUG(200, "the event has passed Dimu selection criteria");
      }
    }
  }
}


bool  PhysicsTriggerModule::eventUserSelect()
{
  B2DEBUG(200, "User custom selection is open");
  const Particle* part = NULL;
  bool Fval = false;
  if (m_cut.check(part))
    Fval = true;
  else
    Fval = false;

  return Fval;
}


void PhysicsTriggerModule::eventInformation()
{
  StoreArray<PhysicsTriggerInformation> phyTriInfos;
  PhysicsTriggerInformation* phyTriInfo = phyTriInfos.appendNew(PhysicsTriggerInformation());
  StoreObjPtr<ParticleList> pionshlt("pi+:HLT");
  for (unsigned int i = 0; i < pionshlt->getListSize(); i++) {
    Particle* pion = pionshlt->getParticle(i);
    m_dr = Variable::particleDRho(pion);
    m_dz = Variable::particleDZ(pion);
    m_pt = Variable::particlePt(pion);
    m_p = Variable::particleP(pion);
    m_costheta = Variable::particleCosTheta(pion);
    m_phi = Variable::particlePhi(pion);
    m_ECLClusterTiming = Variable::ECLClusterTimingHLT(pion);
    m_ECLClusterTheta = Variable::ECLClusterThetaHLT(pion);
    m_ECLClusterE = Variable::ECLClusterEHLT(pion);
    m_Charge = Variable::particleCharge(pion);
    phyTriInfo->setDr(m_dr);
    phyTriInfo->setDz(m_dz);
    phyTriInfo->setPt(m_pt);
    phyTriInfo->setP(m_p);
    phyTriInfo->setCosTheta(m_costheta);
    phyTriInfo->setPhi(m_phi);
    phyTriInfo->setCharge(m_Charge);
    if (m_ECLClusterE == -1.0) continue;
    phyTriInfo->setECLClusterE(m_ECLClusterE);
    phyTriInfo->setECLClusterTheta(m_ECLClusterTheta);
    phyTriInfo->setECLClusterTiming(m_ECLClusterTiming);
    //phyTriInfo->setP(m_p);
    //phyTriInfo->setDr(m_dr);
  }

  StoreObjPtr<ParticleList> gammahlt("gamma:HLT");
  for (unsigned int i = 0; i < gammahlt->getListSize(); i++) {
    Particle* gamma = gammahlt->getParticle(i);
    m_ECLClusterTiming = Variable::ECLClusterTimingHLT(gamma);
    m_ECLClusterTheta = Variable::ECLClusterThetaHLT(gamma);
    m_ECLClusterE = Variable::ECLClusterEHLT(gamma);
    m_Charge = Variable::particleCharge(gamma);
    phyTriInfo->setECLClusterE(m_ECLClusterE);
    phyTriInfo->setECLClusterTheta(m_ECLClusterTheta);
    phyTriInfo->setECLClusterTiming(m_ECLClusterTiming);
    phyTriInfo->setCharge(m_Charge);

  }



  Particle* p = NULL;
  m_ngoodTracks = Variable::nTracksHLT(p);
  m_ngoodECLClusters = Variable::nECLClustersHLT(p);
  m_nKLMClusters = Variable::nKLMClustersHLT(p);
  m_Pzvis = Variable::VisiblePzHLT(p);
  m_Esum = Variable::ECLEnergyHLT(p);
  m_Evis = Variable::VisibleEnergyHLT(p);
  m_p1 = Variable::TrackP1HLT(p);
  m_p2 = Variable::TrackP2HLT(p);
  m_e1 = Variable::ECLClusterE1HLT(p);
  m_e2 = Variable::ECLClusterE2HLT(p);
  m_angleTT = Variable::AngleTTHLT(p);
  m_maxangleTT = Variable::MaxAngleTTHLT(p);
  m_angleGG = Variable::AngleGGHLT(p);

//The total ECL energy
  phyTriInfo->setNGoodTrack(m_ngoodTracks);
  phyTriInfo->setNGoodECLClusters(m_ngoodECLClusters);
  phyTriInfo->setNKLMClusters(m_nKLMClusters);
  phyTriInfo->setEvis(m_Evis);
  phyTriInfo->setPzvis(m_Pzvis);
  phyTriInfo->setEsum(m_Esum);
  phyTriInfo->setAngleTT(m_angleTT);
  phyTriInfo->setMaxAngleTT(m_maxangleTT);
  phyTriInfo->setAngleGG(m_angleGG);
  phyTriInfo->setP1(m_p1);
  phyTriInfo->setP2(m_p2);
  phyTriInfo->setE1(m_e1);
  phyTriInfo->setE2(m_e2);
  phyTriInfo->setUserCustomValue(m_userCustomOpen);
}




void PhysicsTriggerModule::initializeForEvent()
{
  m_summary = 0;
  m_ngoodTracks = 0;
  m_ngoodECLClusters = 0;
  m_nKLMClusters = 0;
  m_Evis = 0.;
  m_Pzvis = 0.;
  m_p = -1.;
  m_costheta = -10.;
  m_phi = -10.;
  m_ECLClusterE = -1.;
  m_ECLClusterTheta = -10.;
  m_ECLClusterPhi = -10.;
  m_ECLClusterTiming = -9999.;
  m_Esum = 0.;
  //m_Elargest = 0.;
  m_angleTT = -10.;
  m_maxangleTT = -10.;
  m_angleGG = -10.;
  m_p1 = -1.;
  m_p2 = -1.;
  m_ip1 = -1;
  m_ip2 = -1;
  m_e1 = -1.;
  m_e2 = -1.;
  m_ie1 = -1;
  m_ie2 = -1;

}

void PhysicsTriggerModule::ReadParameters()
{
  Gearbox& gb = Gearbox::getInstance();
  gb.open("skim/hlt/SelectionCriteria.xml");

  GearDir SelectionCriteria("/HLTSelectionCriteria/Conditions/");
  GearDir SelectionCriteriaContent(SelectionCriteria);
  SelectionCriteriaContent.append((boost::format("Condition[%1%]/") % (m_parameterCutIndex)).str());

//Hadron Criteria
  m_nTrackCutMin_had = SelectionCriteriaContent.getInt("HadronCriteria/GoodTracksMin");
  m_nECLClusterCutMin_had = SelectionCriteriaContent.getInt("HadronCriteria/GoodECLClustersMin");
  m_EvisCutMin_had = SelectionCriteriaContent.getEnergy("HadronCriteria/VisibleEnergyMin");
  m_EsumCutMin_had = SelectionCriteriaContent.getEnergy("HadronCriteria/TotalECLEnergyMin");
  m_EsumCutMax_had = SelectionCriteriaContent.getEnergy("HadronCriteria/TotalECLEnergyMax");

//Tau-pair
  m_nTrackCutMin_tau = SelectionCriteriaContent.getInt("TauCriteria/GoodTracksMin");
  m_nTrackCutMax_tau = SelectionCriteriaContent.getInt("TauCriteria/GoodTracksMax");
  m_EsumCutMax_tau = SelectionCriteriaContent.getEnergy("TauCriteria/VisibleEnergyMax");

//Bhabha
  m_nTrackCutMin_bhabha = SelectionCriteriaContent.getInt("BhabhaCriteria/GoodTracksMin");
  m_nECLClusterCutMin_bhabha = SelectionCriteriaContent.getInt("BhabhaCriteria/GoodECLClustersMin");
  m_EsumCutMin_bhabha = SelectionCriteriaContent.getEnergy("BhabhaCriteria/TotalECLEnergyMin");
  m_ECLClusterE1CutMin_bhabha = SelectionCriteriaContent.getEnergy("BhabhaCriteria/ECLEnergyE1Min");
  m_AngleTTCutMax_bhabha = SelectionCriteriaContent.getAngle("BhabhaCriteria/AngleTTMax");

//Mu-pair
  m_nTrackCutMin_dimu = SelectionCriteriaContent.getInt("DimuCriteria/GoodTracksMin");
  m_EsumCutMin_dimu = SelectionCriteriaContent.getEnergy("DimuCriteria/TotalECLEnergyMin");
  m_AngleTTCutMax_dimu = SelectionCriteriaContent.getAngle("DimuCriteria/AngleTTMax");


//Gamma-pair

  m_nTrackCutMax_digamma = SelectionCriteriaContent.getInt("DigammaCriteria/GoodTracksMax");
  m_nECLClusterCutMin_digamma = SelectionCriteriaContent.getInt("DigammaCriteria/GoodECLClustersMin");
  m_EsumCutMin_digamma = SelectionCriteriaContent.getEnergy("DigammaCriteria/TotalECLEnergyMin");
  m_ECLClusterE1CutMin_digamma = SelectionCriteriaContent.getEnergy("DigammaCriteria/ECLEnergyE1Min");
  m_AngleGGCutMax_digamma = SelectionCriteriaContent.getAngle("DigammaCriteria/AngleGGMax");

//Two-photon

//Random trigger

//Salvage

  gb.close();
  gb.open("geometry/Belle2.xml");
}


