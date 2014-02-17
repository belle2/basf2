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

#include <tracking/cdcLocalTracking/topology/CDCWireTopology.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

CDCMCMap::CDCMCMap()
{
}



CDCMCMap::~CDCMCMap()
{
}



void CDCMCMap::clear()
{

  B2DEBUG(100, "In CDCMCMap::clear()");
  m_simHitByHit.clear();

  m_mcParticlesByHit.clear();
  m_mcParticlesBySimHit.clear();

  m_reassignedSecondaryHits.clear();
  m_reassignedSecondarySimHits.clear();

  B2INFO("m_simHitByHit.size(): " << m_simHitByHit.size());

  B2INFO("m_mcParticlesByHit.size(): " << m_mcParticlesByHit.size());
  B2INFO("m_mcParticlesBySimHit.size(): " << m_mcParticlesBySimHit.size());

  B2INFO("m_reassignedSecondaryHits.size(): " << m_reassignedSecondaryHits.size());
  B2INFO("m_reassignedSecondarySimHits.size(): " << m_reassignedSecondarySimHits.size());


}



void CDCMCMap::fill()
{

  B2DEBUG(100, "In CDCMCMap::fill()");
  clear();

  fillSimHitByHitMap();
  fillMCParticleByHitMap();
  fillMCParticleBySimHitMap();

  validateRelations();
  validateReassignedSecondaries();


  B2INFO("m_simHitByHit.size(): " << m_simHitByHit.size());

  B2INFO("m_mcParticlesByHit.size(): " << m_mcParticlesByHit.size());
  B2INFO("m_mcParticlesBySimHit.size(): " << m_mcParticlesBySimHit.size());

  B2INFO("m_reassignedSecondaryHits.size(): " << m_reassignedSecondaryHits.size());
  B2INFO("m_reassignedSecondarySimHits.size(): " << m_reassignedSecondarySimHits.size());

}



void CDCMCMap::fillSimHitByHitMap()
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
      B2WARNING("CDCHit has no related CDCSimHit in CDCMCMap::fill()");
    };

  }

}






void CDCMCMap::fillMCParticleByHitMap()
{

  StoreArray<MCParticle> mcParticles;
  StoreArray<CDCHit> hits;

  RelationArray mcParticleToHitsRelations(mcParticles, hits);

  //Pickup an iterator for hinted insertion
  MCParticleByCDCHitMap::iterator itInsertHint = m_mcParticlesByHit.end();

  for (const RelationElement & mcParticleToHitsRelation : mcParticleToHitsRelations) {

    RelationElement::index_type iMCParticle = mcParticleToHitsRelation.getFromIndex();
    const MCParticle* ptrMCParticle = mcParticles[iMCParticle];

    size_t nRelatedHits = mcParticleToHitsRelation.getSize();
    for (size_t iRelation = 0; iRelation < nRelatedHits; ++iRelation) {

      RelationElement::index_type iHit = mcParticleToHitsRelation.getToIndex(iRelation);
      RelationElement::weight_type weight = mcParticleToHitsRelation.getWeight(iRelation);

      const CDCHit* ptrHit = hits[iHit];

      if (m_mcParticlesByHit.by<CDCHit>().count(ptrHit) != 0) {
        B2WARNING("CDCHit as more than one related MCParticle - reorganize the mapping");
      }

      if (indicatesReassignedSecondary(weight)) {
        m_reassignedSecondaryHits.insert(ptrHit);
      }

      itInsertHint = m_mcParticlesByHit.insert(itInsertHint, MCParticleByCDCHitMap::value_type(ptrHit, ptrMCParticle));

    }

  }

  //Check if every hit has a corresponding MCParticle
  //Only exception is, if the hit is background.
  for (const CDCHit & hit : hits) {
    const CDCHit* ptrHit = &hit;

    if (m_mcParticlesByHit.by<CDCHit>().count(ptrHit) == 0 and not isBackground(ptrHit)) {
      B2WARNING("CDCHit has no related MCParticle but CDCSimHit indicates that it is no background in CDCMCMap::fill()");
    };

  }

}



void CDCMCMap::fillMCParticleBySimHitMap()
{

  StoreArray<MCParticle> mcParticles;
  StoreArray<CDCSimHit> simHits;

  RelationArray mcParticleToSimHitsRelations(mcParticles, simHits);

  //Pickup an iterator for hinted insertion
  MCParticleByCDCSimHitMap::iterator itInsertHint = m_mcParticlesBySimHit.end();

  for (const RelationElement & mcParticleToSimHitsRelation : mcParticleToSimHitsRelations) {

    RelationElement::index_type iMCParticle = mcParticleToSimHitsRelation.getFromIndex();
    const MCParticle* ptrMCParticle = mcParticles[iMCParticle];

    size_t nRelatedHits = mcParticleToSimHitsRelation.getSize();
    for (size_t iRelation = 0; iRelation < nRelatedHits; ++iRelation) {

      RelationElement::index_type iSimHit = mcParticleToSimHitsRelation.getToIndex(iRelation);
      RelationElement::weight_type weight = mcParticleToSimHitsRelation.getWeight(iRelation);

      const CDCSimHit* ptrSimHit = simHits[iSimHit];

      if (m_mcParticlesBySimHit.by<CDCSimHit>().count(ptrSimHit) != 0) {
        B2WARNING("CDCSimHit as more than one related MCParticle - reorganize the mapping");
      }

      if (indicatesReassignedSecondary(weight)) {
        m_reassignedSecondarySimHits.insert(ptrSimHit);
      }

      itInsertHint = m_mcParticlesBySimHit.insert(itInsertHint, MCParticleByCDCSimHitMap::value_type(ptrSimHit, ptrMCParticle));

    }

  }

  //Check if every hit has a corresponding MCParticle
  //Only exception is, if the hit is background.
  for (const CDCSimHit & simHit : simHits) {
    const CDCSimHit* ptrSimHit = &simHit;

    if (m_mcParticlesBySimHit.by<CDCSimHit>().count(ptrSimHit) == 0 and not isBackground(ptrSimHit)) {
      B2WARNING("CDCSimHit has no related MCParticle but CDCSimHit indicates that it is no background in CDCMCMap::fill()");
    };

  }

}



void CDCMCMap::validateRelations() const
{

  StoreArray<CDCHit> hits;

  for (const CDCHit & hit : hits) {
    const CDCHit* ptrHit = &hit;


    const CDCSimHit* ptrSimHit = getSimHit(ptrHit);
    const MCParticle* ptrMCParticle = getMCParticle(ptrHit);

    const MCParticle* ptrMCParticleFromSimHit = getMCParticle(ptrSimHit);

    if (ptrMCParticle != ptrMCParticleFromSimHit) {
      B2WARNING("MCParticle from CDCHit and MCParticle from related CDCSimHit mismatch in CDCMCMap::validateRelations()");
    }


  }
}



void CDCMCMap::validateReassignedSecondaries() const
{

  for (const CDCHit * ptrHit : m_reassignedSecondaryHits) {

    const CDCSimHit* ptrSimHit = getSimHit(ptrHit);
    if (not isReassignedSecondary(ptrSimHit)) {
      B2WARNING("CDCHit is reassigned secondary but related CDCSimHit is not.");
    }

  }

}
