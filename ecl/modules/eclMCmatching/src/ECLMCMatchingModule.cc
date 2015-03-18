/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen, Benjamin Oberhof                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclMCmatching/ECLMCMatchingModule.h>

//framework headers
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

//ecl package headers
#include <ecl/dataobjects/ECLHit.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLHitAssignment.h>
#include <ecl/dataobjects/ECLGamma.h>
#include <ecl/dataobjects/ECLPi0.h>

#include <mdst/dataobjects/ECLCluster.h>

#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/RelationArray.h>


//root
#include <TVector3.h>

//C++ STL
#include <cstdlib>
#include <iomanip>
#include <math.h>
#include <time.h>
#include <iomanip>
#include <utility> //contains pair

using namespace std;
using namespace boost;
using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLMCMatching)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLMCMatchingModule::ECLMCMatchingModule() : Module()
{
  // Set description

  setDescription("ECLMCMatchingModule");
  setPropertyFlags(c_ParallelProcessingCertified);

}

ECLMCMatchingModule::~ECLMCMatchingModule()
{
}

void ECLMCMatchingModule::initialize()
{
  // Initialize variables
  m_nRun    = 0 ;
  m_nEvent  = 0 ;
  // CPU time start
  m_timeCPU = clock() * Unit::us;
  StoreArray<MCParticle> MCParticleArray;
  StoreArray<ECLHit> ECLHitArray;
  StoreArray<ECLDigit> ECLDigitArray;
  StoreArray<ECLShower> ECLShowerArray;
  StoreArray<ECLCluster> ECLClusterArray;
  MCParticleArray.registerRelationTo(ECLHitArray);
  //MCParticleArray.registerRelationTo(ECLDigitArray);
  ECLDigitArray.registerRelationTo(MCParticleArray);
  ECLShowerArray.registerRelationTo(MCParticleArray);
  ECLClusterArray.registerRelationTo(MCParticleArray);

  StoreArray<ECLSimHit> ECLSimHitArray;
  ECLSimHitArray.registerRelationTo(ECLHitArray);

  // RelationArray::registerPersistent<ECLHit,MCParticle>("", ""); obsolete
  // RelationArray::registerPersistent<ECLDigit,MCParticle>("", ""); obsolete
  // RelationArray::registerPersistent<ECLShower,MCParticle>("", ""); obsolete
  // RelationArray::registerPersistent<ECLCluster,MCParticle>("", ""); obsolete

}

void ECLMCMatchingModule::beginRun()
{
}

void ECLMCMatchingModule::event()
{

  int DigiIndex[8736];
  int DigiOldTrack[8736];
  for (int i = 0; i < 8736; i++) {
    DigiIndex[i] = -1; DigiOldTrack[i] = -1;
  }

  StoreArray<MCParticle> mcParticles;
  PrimaryTrackMap eclPrimaryMap;
  eclPrimaryMap.clear();
  int nParticles = mcParticles.getEntries();
  if (nParticles > 1000)nParticles = 1000; //skip mcParticles from to speed up

  for (int iPart = 0; iPart < nParticles ; iPart++) {
    if (mcParticles[iPart]->getMother() == NULL
        && !mcParticles[iPart]->hasStatus(MCParticle::c_PrimaryParticle)
        && !mcParticles[iPart]->hasStatus(MCParticle::c_StableInGenerator)) continue;

    bool adhoc_StableInGeneratorFlag(mcParticles[iPart]->hasStatus(MCParticle::c_StableInGenerator));
    if (mcParticles[iPart]->hasStatus(MCParticle::c_PrimaryParticle) && adhoc_StableInGeneratorFlag) {
      if (mcParticles[iPart]->getArrayIndex() == -1) {
        eclPrimaryMap.insert(pair<int, int>(iPart, iPart));
      } else {eclPrimaryMap.insert(pair<int, int>(mcParticles[iPart]->getArrayIndex(), mcParticles[iPart]->getArrayIndex()));}
    } else {
      if (mcParticles[iPart]->getMother() == NULL) continue;
      if (eclPrimaryMap.find(mcParticles[iPart]->getMother()->getArrayIndex()) != eclPrimaryMap.end()) {
        eclPrimaryMap.insert(pair<int, int>(mcParticles[iPart]->getArrayIndex(),
                                            eclPrimaryMap[mcParticles[iPart]->getMother()->getArrayIndex()]));
      }//if mother of mcParticles is stored.
    }//if c_StableInGenerator and c_PrimaryParticle
  }//for mcParticles


  StoreArray<ECLHit> eclHitArray;
  RelationArray eclHitRel(mcParticles, eclHitArray);
  StoreArray<ECLDigit> eclDigiArray;
  RelationArray eclDigiToMCPart(eclDigiArray, mcParticles);
  StoreArray<ECLSimHit> eclSimHitArray;
  RelationArray eclHitToSimHit(eclHitArray, eclSimHitArray);
  RelationArray eclSimHitRel(mcParticles, eclSimHitArray);

  for (int ii = 0; ii < eclDigiArray.getEntries(); ii++) {
    ECLDigit* aECLDigi = eclDigiArray[ii];
    float FitEnergy    = (aECLDigi->getAmp()) / 20000.; //ADC count to GeV
    int cId            = (aECLDigi->getCellId() - 1);
    if (FitEnergy < 0.) {
      continue;
    }
    DigiIndex[cId] = ii;
  }

  for (int index = 0; index < eclHitRel.getEntries(); index++) {
    int PrimaryIndex = -1;
    map<int, int>::iterator iter = eclPrimaryMap.find(eclHitRel[index].getFromIndex());
    if (iter != eclPrimaryMap.end()) {
      PrimaryIndex = iter->first; //it's the daughter
    } else continue;

    for (int hit = 0; hit < (int)eclHitRel[index].getToIndices().size(); hit++) {
      ECLHit* aECLHit = eclHitArray[eclHitRel[index].getToIndex(hit)];
      int hitCellId         = aECLHit->getCellId() - 1;
      if (aECLHit->getBackgroundTag() != 0) continue;

      if (DigiIndex[hitCellId] != -1 && DigiOldTrack[hitCellId] != PrimaryIndex) {
        eclDigiToMCPart.add(DigiIndex[hitCellId], PrimaryIndex);
        DigiOldTrack[hitCellId] = PrimaryIndex;
      }
    }//for (int hit = 0
  }//for index


  StoreArray<ECLShower> eclRecShowerArray;
  StoreArray<ECLHitAssignment> eclHitAssignmentArray;
  RelationArray  eclShowerToMCPart(eclRecShowerArray, mcParticles);
  RelationArray  eclShowerToMCParts(eclRecShowerArray, mcParticles);
  map<int, float> mc_relations;

  map<int, float> eclMCParticleContributionMap;;
  eclMCParticleContributionMap.clear();
  mc_relations.clear();

  for (int iShower = 0; iShower < eclRecShowerArray.getEntries(); iShower++) {
    ECLShower* aECLShower = eclRecShowerArray[iShower];
    double showerId = aECLShower->getShowerId();
    for (int iMCPart = 0; iMCPart < eclSimHitRel.getEntries(); iMCPart++) {
      float ene = 0;
      for (int iHA = 0; iHA <  eclHitAssignmentArray.getEntries(); iHA++) {
        ECLHitAssignment* aECLHitAssignment = eclHitAssignmentArray[iHA];
        int m_HAShowerId = aECLHitAssignment->getShowerId();
        int m_HAcellId = aECLHitAssignment->getCellId() - 1 ;
        if (m_HAShowerId != showerId)continue;
        if (m_HAShowerId > showerId)break;

        for (int simhit = 0; simhit < (int)eclSimHitRel[iMCPart].getToIndices().size(); simhit++) {
          ECLSimHit* aECLSimHit = eclSimHitArray[eclSimHitRel[iMCPart].getToIndex(simhit)];
          int hitCellId         = aECLSimHit->getCellId() - 1;
          if ((hitCellId != m_HAcellId)) continue;

          map<int, float>::iterator iter =  eclMCParticleContributionMap.find((int) eclSimHitRel[iMCPart].getToIndex(simhit));
          if (iter == eclMCParticleContributionMap.end()) {
            ene = ene + aECLSimHit->getEnergyDep();
          } else {
            ene = ene + aECLSimHit->getEnergyDep();
          }
        }//for simhit
      }//for hANum //iMCPart
      if (ene > 0.) {
        eclMCParticleContributionMap.insert(pair<int, float>((int) eclSimHitRel[iMCPart].getFromIndex(), ene));
      }
    }//for iMCPart //HA hANum

    int PrimaryIndex = -1;
    int MaxContribution = 0;
    for (map<int, float>::iterator i = eclMCParticleContributionMap.begin(); i != eclMCParticleContributionMap.end(); ++i) {
      mc_relations[(*i).first] += (*i).second;
      if ((*i).second > MaxContribution) {MaxContribution = (*i).second ;  PrimaryIndex = (*i).first ;}
    }

    eclMCParticleContributionMap.clear();

    if (PrimaryIndex == -1) {
    } else {
      for (map<int, float>::iterator i = mc_relations.begin(); i != mc_relations.end(); ++i) {
        eclShowerToMCPart.add(showerId, (*i).first, (*i).second);
      }
    }
    mc_relations.clear();
  }//ShowerNum

  //... Related ECLClustertoMCParticle
  //... First get relation of ECLCluster to ECLShower
  //... Then exploit already available ECLShower relation to MCParticle
  //... get the index of MCParticle from the relation of ECLShower and use it to associate ECLCluster

  StoreArray<ECLCluster> eclClusterArray;
  RelationArray ECLClustertoMCPart(eclClusterArray, mcParticles);

  for (int imdst = 0; imdst < eclClusterArray.getEntries(); ++imdst) {
    const ECLCluster* cluster = eclClusterArray[imdst];
    for (auto eclShower : cluster->getRelationsTo<ECLShower>()) {
      ECLShower* veclShower = eclRecShowerArray[eclShower.getShowerId()];
      for (auto MCpart : veclShower->getRelationsTo<MCParticle>()) {
        for (int i = 0; i < eclShowerToMCPart.getEntries(); i++) {
          if ((int)eclShowerToMCPart[i].getFromIndex() == eclShower.getShowerId()
              && (int)eclShowerToMCPart[i].getToIndex() == MCpart.getArrayIndex()) {
            //cout << "From: " << (int)eclShowerToMCPart[i].getFromIndex() << " To: " << (int)eclShowerToMCPart[i].getToIndex() << endl;
            ECLClustertoMCPart.add(imdst, MCpart.getArrayIndex(), eclShowerToMCPart[i].getWeight());
          }
        }
      }
    }
  } // imdst

  for (int ii = 0; ii < ECLClustertoMCPart.getEntries(); ii++) {
    //cout << "ClusterID: " << ECLClustertoMCPart[ii].getFromIndex() << " Energy: " <<  eclClusterArray[ECLClustertoMCPart[ii].getFromIndex()]->getEnergy() << " MCParticle: " << ECLClustertoMCPart[ii].getToIndex() <<  " Weight: " << ECLClustertoMCPart[ii].getWeight() << endl;
  }
  m_nEvent++;
}


void ECLMCMatchingModule::endRun()
{
  m_nRun++;
}

void ECLMCMatchingModule::terminate()
{
}
