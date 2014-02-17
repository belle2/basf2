/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCMCMap.h"

#include <framework/datastore/RelationVector.h>
#include <tracking/cdcLocalTracking/topology/CDCWireTopology.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

//ClassImpInCDCLocalTracking(CDCMCMap)

namespace {
  CDCMCMap* g_mcMap = nullptr;
}



CDCMCMap::CDCMCMap()
{
}



CDCMCMap::~CDCMCMap()
{
}



CDCMCMap& CDCMCMap::getInstance()
{
  if (not g_mcMap) g_mcMap = new CDCMCMap;
  return *g_mcMap;
}



void CDCMCMap::clear()
{

  m_simHitByHit.clear();

  //m_mcParticlesByHit.clear();
  //m_mcParticlesBySimHit.clear();

  m_reassignedSecondaryHits.clear();
  m_reassignedSecondaryHits.clear();

}



void CDCMCMap::fill()
{

  fillHitToSimHitMap();


  /* B2DEBUG(100, "In CDCMCMap::fill()"); */
  /* clear(); */

  /* const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance(); */

  /* std::vector< std::pair<int, int> > mcParticleIdx_simHitTrackId_pairs; */

  /* for (const CDCWireHit & wireHit : wireHitTopology.getWireHits()) { */

  /*   const CDCHit* ptrHit = wireHit.getHit(); */
  /*   if (not ptrHit) { */
  /*     B2ERROR("CDCWireHit has no containing CDCHit in CDCMCMap::fill()"); */
  /*     continue; */
  /*   } */
  /*   const CDCHit& hit = *ptrHit; */



  /*   const CDCSimHit* ptrSimHit = hit.getRelated<CDCSimHit>(); */
  /*   if (not ptrSimHit) { */
  /*     B2ERROR("CDCHit has no related CDCSimHit in CDCMCMap::fill()"); */
  /*     continue; */
  /*   } */
  /*   const CDCSimHit& simHit = *ptrSimHit; */



  /*   const RelationVector<MCParticle> mcParticles = hit.getRelationsWith<MCParticle>(); */

  /*   if (mcParticles.size() > 1) { */
  /*     B2ERROR("CDCHit has more than one related MCParticle in CDCMCMap::fill()"); */
  /*     continue; */
  /*   } else if (mcParticles.size() == 0) { */
  /*     //CDCHit is background */
  /*     //Check if background flag is set */
  /*     if (simHit.getBackgroundTag() == CDCSimHit::bg_none) { */
  /*       B2ERROR("CDCHit has no MCParticle but the related CDCSimHit is not marked as background."); */
  /*     } */
  /*     continue; //for CDCWireHit */
  /*   } */
  /*   const MCParticle* ptrMCParticle = mcParticles.object(0); */
  /*   const MCParticle& mcParticle = *ptrMCParticle; */
  /*   double mcRelationWeight = mcParticles.weight(0); */



  /*   const RelationVector<MCParticle> mcParticlesFromSimHit = simHit.getRelationsWith<MCParticle>(); */

  /*   if (mcParticlesFromSimHit.size() > 1) { */
  /*     B2ERROR("CDCSimHit has more than one related MCParticle in CDCMCMap::fill()"); */
  /*     continue; */

  /*   } else if (mcParticlesFromSimHit.size() == 0) { */
  /*     B2ERROR("CDCSimHit has no related MCParticle but the CDCHit as a related MCParticle in CDCMCMap::fill()"); */
  /*     continue; */

  /*   } */
  /*   const MCParticle* ptrMCParticleFromSimHit = mcParticlesFromSimHit.object(0); */
  /*   double mcRelationWeightFromSimHit =  mcParticlesFromSimHit.weight(0); */

  /*   if (ptrMCParticle != ptrMCParticleFromSimHit) { */
  /*     B2ERROR("MCParticle from CDCSimHit and CDCHit mismatch in CDCMCMap::fill()"); */
  /*     continue; */
  /*   } */

  /*   if (mcRelationWeight != mcRelationWeightFromSimHit) { */
  /*     B2WARNING("The relation weights from SimHit and CDCHit to MCParticle mismatch in CDCMCMap::fill()"); */
  /*     B2WARNING("mcRelationWeight: " << mcRelationWeight); */
  /*     B2WARNING("mcRelationWeightFromSimHit: " << mcRelationWeightFromSimHit); */
  /*     if (mcRelationWeight * mcRelationWeightFromSimHit < 0) { */
  /*       B2ERROR("The relation weights from SimHit and CDCHit to MCParticle have mismatching signs in CDCMCMap::fill()"); */
  /*       continue; */
  /*     } */
  /*   } */



  /*   int mcParticleIdx = mcParticle.getArrayIndex(); */

  /*   int simHitTrackId = simHit.getTrackId(); */
  /*   mcParticleIdx_simHitTrackId_pairs.emplace_back(simHitTrackId, mcParticleIdx); */

  /*   //Also consider */
  /*   //getStatus, getSecondaryPhysicsProcess */
  /*   //for additional information */

  /* } //end for wire hits */



  /* // Look at the relation of the MCParticle::getArrayIndex() and CDCSimHit::getTrackId() */
  /* std::sort(mcParticleIdx_simHitTrackId_pairs.begin(), mcParticleIdx_simHitTrackId_pairs.end()); */

  /* mcParticleIdx_simHitTrackId_pairs.erase( */
  /*   std::unique(mcParticleIdx_simHitTrackId_pairs.begin(), */
  /*               mcParticleIdx_simHitTrackId_pairs.end()), */
  /*   mcParticleIdx_simHitTrackId_pairs.end()); */

  /* for (const pair<int, int> mcParticleIdx_simHitTrackId_pair : mcParticleIdx_simHitTrackId_pairs) { */

  /*   B2DEBUG(100, "mcParticleIdx <-> simHitTrackId: " <<  mcParticleIdx_simHitTrackId_pair.first << " <-> " << mcParticleIdx_simHitTrackId_pair.second); */

  /* } */

}



void CDCMCMap::fillHitToSimHitMap()
{

  StoreArray<CDCSimHit> simHits;
  StoreArray<CDCHit> hits;

  RelationArray simhitToHitsRelations(simHits, hits);

  //Pickup an iterator for hinted insertion
  CDCSimHitByCDCHitMap::iterator itInsertHint = m_simHitByHit.end();

  for (const RelationElement & simHitToHitsRelation : simhitToHitsRelations) {

    RelationElement::index_type iSimHit = simHitToHitsRelation.getFromIndex();
    const CDCSimHit* ptrSimHit = simHits[iSimHit];

    size_t nRelatedHits = simHitToHitsRelation.getSize();
    for (size_t iRelation = 0; iRelation < nRelatedHits; ++iRelation) {

      RelationElement::index_type iHit = simHitToHitsRelation.getToIndex(iRelation);
      //RelationElement::weight_type weight = simHitToHitsRelation.getWeight(iRelation);

      const CDCHit* ptrHit = hits[iHit];
      if (m_simHitByHit.by<CDCHit>().count(ptrHit) != 0) {
        B2WARNING("CDCHit as more than one related CDCSimHit - reorganize the mapping");
      }

      if (m_simHitByHit.by<CDCSimHit>().count(ptrSimHit) != 0) {
        B2WARNING("CDCSimHit as more than one related CDCHit - reorganize the mapping");
      }

      itInsertHint = m_simHitByHit.insert(itInsertHint, CDCSimHitByCDCHitMap::value_type(ptrHit, ptrSimHit));

    }

  }

  //Check if every hit has a corresponding simhit
  for (const CDCHit & hit : hits) {
    const CDCHit* ptrHit = &hit;

    if (m_simHitByHit.by<CDCHit>().count(ptrHit) == 0) {
      B2ERROR("CDCHit has no related CDCSimHit in CDCMCMap::fill()");
    };

  }

}



/*
for (const CDCWireHit & wirehit : wirehits) {
size_t iStoredHit = wirehit.getStoreIHit();

bool found = false;
for (int iRelation = 0; iRelation < nRelation and not found; ++iRelation) {
  const RelationElement& relationElement = simhitsToHitsRelation[iRelation];

      if (relationElement.getSize() > 1)
        B2WARNING("Rework CDCMCLookUp! CDCSimHits to CDCHit correspondence is not unique. ( Maybe double hit semantics have been introduced  ?)");

      RelationElement::index_type iStoredHitInRelation = relationElement.getToIndex(0);
      if (iStoredHitInRelation == iStoredHit) {
        found = true;
        RelationElement::index_type iStoredSimHit = relationElement.getFromIndex();
        CDCSimHit* simHit = storedSimhits[iStoredSimHit];

        addSimHit(&wirehit, simHit, iStoredSimHit);

        // We use both time of flight and index of the stored simhit
        // There used to be simhits that had the same time of flight to them
        // So the only way to distinguish the order of their occurance was by the index
        // in their StoreArray
        FlightTimeAndIndex flightTimeAndIndex(simHit->getFlightTime(), iStoredSimHit);
        m_timeSortedWireHits[flightTimeAndIndex] = &wirehit;

      }
    } // end for iRelation

    if (found == false) {
      B2WARNING("No CDCSimHit found for the give CDCHit");
      std::cin >> found;
    }

  } // end for itWireHit

  }*/
