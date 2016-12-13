/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Thomas Hauth                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/mcMatcher/TrackMatchLookUp.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationEntry.h>
#include <framework/datastore/DataStore.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;

TrackMatchLookUp::TrackMatchLookUp(const std::string& mcRecoTrackStoreArrayName,
                                   const std::string& prRecoTrackStoreArrayName)
  : m_mcTracksStoreArrayName(DataStore::arrayName<RecoTrack>(mcRecoTrackStoreArrayName)),
    m_prTracksStoreArrayName(DataStore::arrayName<RecoTrack>(prRecoTrackStoreArrayName))
{
  if (m_mcTracksStoreArrayName == m_prTracksStoreArrayName) {
    B2WARNING("Pattern recognition and Monte Carlo track candidate StoreArray are the same.");
  }
}


TrackMatchLookUp::~TrackMatchLookUp()
{
}



bool TrackMatchLookUp::isMCTrackCand(const RecoTrack& trackCand)
{
  return trackCand.getArrayName() == getMCTracksStoreArrayName();
}



bool TrackMatchLookUp::isPRTrackCand(const RecoTrack& trackCand)
{
  return trackCand.getArrayName() == getPRTracksStoreArrayName();
}




const RecoTrack* TrackMatchLookUp::getRelatedMCTrackCand(const RecoTrack& prRecoTrack, float& purity)
{
  std::pair<RecoTrack*, double> mcRecoTrackAndWeight = prRecoTrack.getRelatedWithWeight<RecoTrack>(getMCTracksStoreArrayName());
  const RecoTrack* mcRecoTrack = mcRecoTrackAndWeight.first;
  if (mcRecoTrack) {
    purity = mcRecoTrackAndWeight.second;
  } else {
    purity = NAN;
  }
  return mcRecoTrack;
}





const RecoTrack* TrackMatchLookUp::getRelatedPRTrackCand(const RecoTrack& mcRecoTrack, float& efficiency)
{
  std::pair<RecoTrack*, double> prRecoTrackAndWeight = mcRecoTrack.getRelatedWithWeight<RecoTrack>(getPRTracksStoreArrayName());
  const RecoTrack* prRecoTrack = prRecoTrackAndWeight.first;
  if (prRecoTrack) {
    efficiency = prRecoTrackAndWeight.second;
  } else {
    efficiency = NAN;
  }
  return prRecoTrack;
}




//Helper functions to figure out the match of the PRParticle
TrackMatchLookUp::MCToPR::MatchInfo
TrackMatchLookUp::extractMCToPRMatchInfo(const RecoTrack* prRecoTrack, const float& efficiency)
{
  if (!prRecoTrack) return MCToPR::MatchInfo::MISSING;
  if (efficiency < 0) return MCToPR::MatchInfo::MERGED;
  else if (efficiency > 0) return MCToPR::MatchInfo::MATCHED;

  return MCToPR::MatchInfo::INVALID;
}

TrackMatchLookUp::PRToMC::MatchInfo
TrackMatchLookUp::extractPRToMCMatchInfo(const RecoTrack& prRecoTrack, const RecoTrack* mcRecoTrack,
                                         const float& purity)
{

  if (!mcRecoTrack) {

    // The patter recognition track has no associated Monte Carlo track.
    // Figure out of it is a clone or a match by the McTrackId property assigned to the track cand.
    // That is also why we need the pattern recogntion track in this method as well.
    const RecoTrack::MatchingStatus matchingStatus = prRecoTrack.getMatchingStatus();

    if (matchingStatus == RecoTrack::MatchingStatus::c_ghost) return PRToMC::MatchInfo::GHOST;
    else if (matchingStatus == RecoTrack::MatchingStatus::c_background) return PRToMC::MatchInfo::BACKGROUND;
    else if (matchingStatus == RecoTrack::MatchingStatus::c_clone) return
        PRToMC::MatchInfo::CLONE; // MCTrackMatcher is running without RelateClonesToMcParticles
    else if (matchingStatus == RecoTrack::MatchingStatus::c_undefined) return PRToMC::MatchInfo::INVALID;

  } else {

    // The patter recognition track has an associated Monte Carlo track.
    // Figure out of it is a clone or a match by the sign of the purity.
    if (purity < 0) return PRToMC::MatchInfo::CLONE;
    else if (purity > 0) return PRToMC::MatchInfo::MATCHED;
    else return PRToMC::MatchInfo::INVALID;

  }

  return PRToMC::MatchInfo::INVALID;
}



const MCParticle*
TrackMatchLookUp::getRelatedMCParticle(const RecoTrack& prRecoTrack)
{
  return prRecoTrack.getRelated<MCParticle>();
}



const RecoTrack*
TrackMatchLookUp::getRelatedMCTrackCand(const RecoTrack& prRecoTrack)
{
  return prRecoTrack.getRelated<RecoTrack>(getMCTracksStoreArrayName());
}



const TrackFitResult*
TrackMatchLookUp::getRelatedTrackFitResult(const RecoTrack& prRecoTrack, Const::ChargedStable chargedStable)
{
  Belle2::Track* b2track = prRecoTrack.getRelated<Belle2::Track>();
  if (b2track)
    // query the Belle2::Track for the selected fit hypothesis
    return b2track->getTrackFitResult(chargedStable);
  else
    return nullptr;
}



const RecoTrack*
TrackMatchLookUp::getRelatedPRTrackCand(const RecoTrack& mcRecoTrack)
{
  return mcRecoTrack.getRelated<RecoTrack>(getPRTracksStoreArrayName());
}



float TrackMatchLookUp::getRelatedPurity(const RecoTrack& prRecoTrack)
{
  float purity = NAN;
  getRelatedMCTrackCand(prRecoTrack, purity);
  return std::fabs(purity);
}



float TrackMatchLookUp::getRelatedEfficiency(const RecoTrack& mcRecoTrack)
{
  float efficiency = NAN;
  getRelatedPRTrackCand(mcRecoTrack, efficiency);
  return std::fabs(efficiency);
}



const RecoTrack*
TrackMatchLookUp::getMatchedMCTrackCand(const RecoTrack& prRecoTrack)
{
  float purity = NAN;
  const RecoTrack* mcRecoTrack = getRelatedMCTrackCand(prRecoTrack, purity);

  if (extractPRToMCMatchInfo(prRecoTrack, mcRecoTrack, purity) == PRToMC::MATCHED) {
    return mcRecoTrack;

  } else {
    return nullptr;

  }
}



const RecoTrack*
TrackMatchLookUp::getMatchedPRTrackCand(const RecoTrack& mcRecoTrack)
{
  float efficiency = NAN;
  const RecoTrack* prRecoTrack = getRelatedPRTrackCand(mcRecoTrack, efficiency);

  if (extractMCToPRMatchInfo(prRecoTrack, efficiency) == MCToPR::MATCHED) {
    return prRecoTrack;

  } else {
    return nullptr;

  }
}



float TrackMatchLookUp::getMatchedPurity(const RecoTrack& trackCand)
{
  if (isMCTrackCand(trackCand)) {
    const RecoTrack& mcRecoTrack = trackCand;
    const RecoTrack* prRecoTrack = getMatchedPRTrackCand(mcRecoTrack);
    if (prRecoTrack) {
      return getRelatedPurity(*prRecoTrack);

    } else {
      return NAN;

    }

  } else {
    const RecoTrack& prRecoTrack = trackCand;
    return getRelatedPurity(prRecoTrack);

  }
}


float TrackMatchLookUp::getMatchedEfficiency(const RecoTrack& trackCand)
{
  if (isPRTrackCand(trackCand)) {
    const RecoTrack& prRecoTrack = trackCand;
    const RecoTrack* mcRecoTrack = getMatchedMCTrackCand(prRecoTrack);
    if (mcRecoTrack) {
      return getRelatedEfficiency(*mcRecoTrack);

    } else {
      return NAN;

    }

  } else {
    const RecoTrack& mcRecoTrack = trackCand;
    return getRelatedPurity(mcRecoTrack);

  }
}
