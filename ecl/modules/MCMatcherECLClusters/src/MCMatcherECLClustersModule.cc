/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Oberhof                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/MCMatcherECLClusters/MCMatcherECLClustersModule.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <ecl/dataobjects/ECLHit.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLHitAssignment.h>

#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/MCParticle.h>

#include <TVector3.h>

#include <cstdlib>
#include <iomanip>
#include <math.h>
#include <time.h>
#include <iomanip>
#include <utility>
#include <algorithm>
#include <vector>

using namespace std;
using namespace boost;
using namespace Belle2;
using namespace ECL;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(MCMatcherECLClusters)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

MCMatcherECLClustersModule::MCMatcherECLClustersModule() : Module()
{
  // Set description

  setDescription("MCMatcherECLClustersModule");
  setPropertyFlags(c_ParallelProcessingCertified);

}

MCMatcherECLClustersModule::~MCMatcherECLClustersModule()
{
}

void MCMatcherECLClustersModule::initialize()
{
  // Initialize variables
  m_nRun    = 0 ;
  // CPU time start
  m_timeCPU = clock() * Unit::us;
  StoreArray<MCParticle> mcParticles;
  StoreArray<ECLHit> eclHits;
  StoreArray<ECLCalDigit> eclCalDigits;
  StoreArray<ECLShower> eclShowers;
  StoreArray<ECLCluster> eclClusters;
  mcParticles.registerRelationTo(eclHits);
  eclCalDigits.registerRelationTo(mcParticles);
  eclShowers.registerRelationTo(mcParticles);
  eclClusters.registerRelationTo(mcParticles);
  StoreArray<ECLSimHit> ECLSimHitArray;
  ECLSimHitArray.registerRelationTo(eclHits);
}

void MCMatcherECLClustersModule::beginRun()
{
}

void MCMatcherECLClustersModule::event()
{
  StoreArray<MCParticle> mcParticles;
  StoreArray<ECLHit> eclHits;
  StoreArray<ECLCalDigit> eclCalDigits;
  StoreArray<ECLSimHit> eclSimHits;
  RelationArray mcParticleToECLHitRelationArray(mcParticles, eclHits);
  RelationArray eclCalDigitToMCParticleRelationArray(eclCalDigits, mcParticles);
  RelationArray eclHitToSimHitRelationArray(eclHits, eclSimHits);
  RelationArray mcParticleToECLSimHitRelationArray(mcParticles, eclSimHits);

  /****************************************************************************************/

  //std::vector<int> DigiIndex(8736);
  int DigiIndex[8736];
  int DigiOldTrack[8736];
  std::fill_n(DigiIndex, 8736, -1);
  std::fill_n(DigiOldTrack, 8736, -1);

  for (const auto& eclCalDigit : eclCalDigits) {
//    float fitEnergy    = (eclDigit.getAmp()) / 20000.; //ADC count to GeV
    float calEnergy    = eclCalDigit.getEnergy(); // Calibrated Energy in GeV
    int cId            = (eclCalDigit.getCellId() - 1);

    if (calEnergy < 0.) {
      continue;
    }
    DigiIndex[cId] = eclCalDigit.getArrayIndex();
  }

  PrimaryTrackMap eclPrimaryMap;  // map<int, int>

  for (const auto& mcParticle : mcParticles) {
    if (mcParticle.getMother() == NULL) continue;

    const int mcParticleIndex = mcParticle.getArrayIndex();
    const int mcParticleMotherIndex = mcParticle.getMother()->getArrayIndex();

    if (mcParticle.hasStatus(MCParticle::c_PrimaryParticle) and mcParticle.hasStatus(MCParticle::c_StableInGenerator)) {
      eclPrimaryMap.insert(pair<int, int>(mcParticleIndex, mcParticleIndex));
    } else if (eclPrimaryMap.find(mcParticle.getMother()->getArrayIndex()) != eclPrimaryMap.end()) {
      eclPrimaryMap.insert(pair<int, int>(mcParticleIndex, eclPrimaryMap[mcParticleMotherIndex]));
    }
  } // mcParticles

  for (int index = 0; index < mcParticleToECLHitRelationArray.getEntries(); index++) {
    int PrimaryIndex = -1;
    const int mcParticleIndex = mcParticleToECLHitRelationArray[index].getFromIndex();
    const map<int, int>::iterator iter = eclPrimaryMap.find(mcParticleIndex);

    if (iter != eclPrimaryMap.end()) {
      PrimaryIndex = iter->first; //it's the daughter
    } else continue;

    for (int hit = 0; hit < (int)mcParticleToECLHitRelationArray[index].getToIndices().size(); hit++) {
      const int eclHitIndex = mcParticleToECLHitRelationArray[index].getToIndex(hit);
      const ECLHit* aECLHit = eclHits[eclHitIndex];
      int hitCellId         = aECLHit->getCellId() - 1;
      if (aECLHit->getBackgroundTag() != 0) continue;

      if (DigiIndex[hitCellId] != -1 && DigiOldTrack[hitCellId] != PrimaryIndex) {
        eclCalDigitToMCParticleRelationArray.add(DigiIndex[hitCellId], PrimaryIndex);
        DigiOldTrack[hitCellId] = PrimaryIndex;
      }
    }//for (int hit = 0
  }//for index

  /****************************************************************************************/

  StoreArray<ECLShower> eclShowers;
  StoreArray<ECLHitAssignment> eclHitAssignments;
  RelationArray eclShowerToMCPart(eclShowers, mcParticles);

  map<int, double> mc_relations;
  map<int, double> eclMCParticleContributionMap;;

  for (const auto& eclShower : eclShowers) {
    const auto showerId = eclShower.getShowerId();
    for (int iMCPart = 0; iMCPart < mcParticleToECLSimHitRelationArray.getEntries(); iMCPart++) {
      double energy = 0;
      for (const auto& eclHitAssignment : eclHitAssignments) {
        const int m_HAShowerId = eclHitAssignment.getShowerId();
        if (m_HAShowerId != showerId) continue;
        if (m_HAShowerId > showerId) break;

        for (int simhit = 0; simhit < (int)mcParticleToECLSimHitRelationArray[iMCPart].getToIndices().size(); simhit++) {
          ECLSimHit* aECLSimHit = eclSimHits[mcParticleToECLSimHitRelationArray[iMCPart].getToIndex(simhit)];
          if ((aECLSimHit->getCellId() - 1 != eclHitAssignment.getCellId() - 1)) continue;
          energy = energy + aECLSimHit->getEnergyDep();
        } // simhit
      } // eclHitAssignment
      if (energy > 0.) {
        eclMCParticleContributionMap.insert(pair<int, double>((int) mcParticleToECLSimHitRelationArray[iMCPart].getFromIndex(), energy));
      }
    }//for iMCPart

    int PrimaryIndex = -1;
    int MaxContribution = 0;
    for (const auto& pair : eclMCParticleContributionMap) {
      mc_relations[pair.first] += pair.second;
      if (pair.second > MaxContribution) {
        MaxContribution = pair.second;
        PrimaryIndex = pair.first;
      }
    }
    eclMCParticleContributionMap.clear();

    if (PrimaryIndex == -1) { //continue?
    } else {
      for (const auto& pair : mc_relations) {
        eclShowerToMCPart.add(showerId, pair.first, pair.second);
      }
    }
    mc_relations.clear();
  } // eclShower

  /****************************************************************************************/

  // to create the relation between ECLCluster->MCParticle with the same weight as
  // the relation between ECLShower->MCParticle.  StoreArray<ECLCluster> eclClusters;
  StoreArray<ECLCluster> eclClusters;
  for (const auto& eclShower : eclShowers) {
    const auto eclCluster = eclShower.getRelatedFrom<ECLCluster>();
    const auto mcParticles = eclShower.getRelationsTo<MCParticle>();
    for (unsigned int i = 0; i < mcParticles.size(); ++i) {
      const auto mcParticle = mcParticles.object(i);
      const auto weight = mcParticles.weight(i);
      eclCluster->addRelationTo(mcParticle, weight);
    }
  }

  for (const auto& eclCluster : eclClusters) {
    const auto mcParticleWeightPair = eclCluster.getRelatedToWithWeight<MCParticle>();
    B2DEBUG(200, "ClusterID: " << eclCluster.getArrayIndex()
            << " Energy: " <<  eclCluster.getEnergy()
            << " MCParticle: " << mcParticleWeightPair.first->getArrayIndex()
            << " Weight: " << mcParticleWeightPair.second);
  }
  B2DEBUG(200, "-------------------------");
}


void MCMatcherECLClustersModule::endRun()
{
  m_nRun++;
}

void MCMatcherECLClustersModule::terminate()
{
}
