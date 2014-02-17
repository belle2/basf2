/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCMCHitLookUp.h"

#include <framework/datastore/RelationVector.h>
#include <tracking/cdcLocalTracking/topology/CDCWireTopology.h>

#include <framework/datastore/StoreArray.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

//ClassImpInCDCLocalTracking(CDCMCHitLookUp)

namespace {
  CDCMCHitLookUp* g_mcHitLookUp = nullptr;
}



CDCMCHitLookUp::CDCMCHitLookUp()
{
}



CDCMCHitLookUp::~CDCMCHitLookUp()
{
}



CDCMCHitLookUp& CDCMCHitLookUp::getInstance()
{
  if (not g_mcHitLookUp) g_mcHitLookUp = new CDCMCHitLookUp;
  return *g_mcHitLookUp;
}



void CDCMCHitLookUp::clear()
{

}



void CDCMCHitLookUp::fill()
{

  B2DEBUG(100, "In CDCMCHitLookUp::fill()");
  clear();

  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

  std::vector< std::pair<int, int> > mcParticleIdx_simHitTrackId_pairs;

  for (const CDCWireHit & wireHit : wireHitTopology.getWireHits()) {

    const CDCHit* ptrHit = wireHit.getHit();
    if (not ptrHit) {
      B2ERROR("CDCWireHit has no containing CDCHit in CDCMCHitLookUp::fill()");
      continue;
    }
    const CDCHit& hit = *ptrHit;



    const CDCSimHit* ptrSimHit = hit.getRelated<CDCSimHit>();
    if (not ptrSimHit) {
      B2ERROR("CDCHit has no related CDCSimHit in CDCMCHitLookUp::fill()");
      continue;
    }
    const CDCSimHit& simHit = *ptrSimHit;



    const RelationVector<MCParticle> mcParticles = hit.getRelationsWith<MCParticle>();

    if (mcParticles.size() > 1) {
      B2ERROR("CDCHit has more than one related MCParticle in CDCMCHitLookUp::fill()");
      continue;
    } else if (mcParticles.size() == 0) {
      //CDCHit is background
      //Check if background flag is set
      if (simHit.getBackgroundTag() == CDCSimHit::bg_none) {
        B2ERROR("CDCHit has no MCParticle but the related CDCSimHit is not marked as background.");
      }
      continue; //for CDCWireHit
    }
    const MCParticle* ptrMCParticle = mcParticles.object(0);
    const MCParticle& mcParticle = *ptrMCParticle;
    double mcRelationWeight = mcParticles.weight(0);



    const RelationVector<MCParticle> mcParticlesFromSimHit = simHit.getRelationsWith<MCParticle>();

    if (mcParticlesFromSimHit.size() > 1) {
      B2ERROR("CDCSimHit has more than one related MCParticle in CDCMCHitLookUp::fill()");
      continue;

    } else if (mcParticlesFromSimHit.size() == 0) {
      B2ERROR("CDCSimHit has no related MCParticle but the CDCHit as a related MCParticle in CDCMCHitLookUp::fill()");
      continue;

    }
    const MCParticle* ptrMCParticleFromSimHit = mcParticlesFromSimHit.object(0);
    double mcRelationWeightFromSimHit =  mcParticlesFromSimHit.weight(0);

    if (ptrMCParticle != ptrMCParticleFromSimHit) {
      B2ERROR("MCParticle from CDCSimHit and CDCHit mismatch in CDCMCHitLookUp::fill()");
      continue;
    }

    if (mcRelationWeight != mcRelationWeightFromSimHit) {
      B2WARNING("The relation weights from SimHit and CDCHit to MCParticle mismatch in CDCMCHitLookUp::fill()");
      B2WARNING("mcRelationWeight: " << mcRelationWeight);
      B2WARNING("mcRelationWeightFromSimHit: " << mcRelationWeightFromSimHit);
      if (mcRelationWeight * mcRelationWeightFromSimHit < 0) {
        B2ERROR("The relation weights from SimHit and CDCHit to MCParticle have mismatching signs in CDCMCHitLookUp::fill()");
        continue;
      }
    }



    int mcParticleIdx = mcParticle.getArrayIndex();

    int simHitTrackId = simHit.getTrackId();
    mcParticleIdx_simHitTrackId_pairs.emplace_back(simHitTrackId, mcParticleIdx);

    //Also consider
    //getStatus, getSecondaryPhysicsProcess
    //for additional information

  } //end for wire hits



  // Look at the relation of the MCParticle::getArrayIndex() and CDCSimHit::getTrackId()
  std::sort(mcParticleIdx_simHitTrackId_pairs.begin(), mcParticleIdx_simHitTrackId_pairs.end());

  mcParticleIdx_simHitTrackId_pairs.erase(
    std::unique(mcParticleIdx_simHitTrackId_pairs.begin(),
                mcParticleIdx_simHitTrackId_pairs.end()),
    mcParticleIdx_simHitTrackId_pairs.end());

  for (const pair<int, int> mcParticleIdx_simHitTrackId_pair : mcParticleIdx_simHitTrackId_pairs) {

    B2DEBUG(100, "mcParticleIdx <-> simHitTrackId: " <<  mcParticleIdx_simHitTrackId_pair.first << " <-> " << mcParticleIdx_simHitTrackId_pair.second);

  }

}




bool CDCMCHitLookUp::isReassignedSecondaryHit(const CDCWireHit& wireHit) const
{
  const CDCHit* hit = wireHit.getHit();
  return CDCSimHitLookUp::getInstance().isReassignedSecondaryHit(hit);
}



const CDCSimHit* CDCMCHitLookUp::getClosestPrimarySimHit(const CDCWireHit& wireHit) const
{
  const CDCHit* hit = wireHit.getHit();
  return CDCSimHitLookUp::getInstance().getClosestPrimarySimHit(hit);
}



RightLeftInfo CDCMCHitLookUp::getRLInfo(const CDCWireHit& wireHit) const
{

  const CDCHit* hit = wireHit.getHit();
  return CDCSimHitLookUp::getInstance().getRLInfo(hit);

}



bool CDCMCHitLookUp::isBackground(const CDCWireHit& wireHit) const
{
  const CDCHit* hit = wireHit.getHit();
  const CDCSimHit* simHit = hit->getRelated<CDCSimHit>();
  return simHit->getBackgroundTag() != CDCSimHit::bg_none;
}



ITrackType CDCMCHitLookUp::getMCTrackId(const CDCWireHit& wireHit) const
{
  const CDCHit* hit = wireHit.getHit();
  const MCParticle* mcParticle = hit->getRelated<MCParticle>();
  return mcParticle ? mcParticle->getArrayIndex() : INVALID_ITRACK;
}



int CDCMCHitLookUp::getInTrackId(const CDCWireHit& wireHit) const
{

  const CDCHit* hit = wireHit.getHit();
  return CDCMCTrackStore::getInstance().getInTrackId(hit);

}



int CDCMCHitLookUp::getInTrackSegmentId(const CDCWireHit& wireHit) const
{

  const CDCHit* hit = wireHit.getHit();
  return CDCMCTrackStore::getInstance().getInTrackSegmentId(hit);

}



int CDCMCHitLookUp::getNPassedSuperLayers(const CDCWireHit& wireHit) const
{

  const CDCHit* hit = wireHit.getHit();
  return CDCMCTrackStore::getInstance().getNPassedSuperLayers(hit);

}
