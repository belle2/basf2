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

#include <mdst/dataobjects/Track.h>

#include <framework/datastore/DataStore.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;

TrackMatchLookUp::TrackMatchLookUp(const std::string& mcRecoTrackStoreArrayName,
                                   const std::string& prRecoTrackStoreArrayName)
  : m_mcTracksStoreArrayName(DataStore::arrayName<RecoTrack>(mcRecoTrackStoreArrayName))
  , m_prTracksStoreArrayName(DataStore::arrayName<RecoTrack>(prRecoTrackStoreArrayName))
{
  if (m_mcTracksStoreArrayName == m_prTracksStoreArrayName) {
    B2WARNING("Pattern recognition and Monte Carlo track StoreArray are the same.");
  }
}

bool TrackMatchLookUp::isMCRecoTrack(const RecoTrack& recoTrack) const
{
  return recoTrack.getArrayName() == getMCTracksStoreArrayName();
}

bool TrackMatchLookUp::isPRRecoTrack(const RecoTrack& recoTrack) const
{
  return recoTrack.getArrayName() == getPRTracksStoreArrayName();
}

const RecoTrack*
TrackMatchLookUp::getRelatedMCRecoTrack(const RecoTrack& prRecoTrack, float& purity) const
{
  assert(isPRRecoTrack(prRecoTrack));
  std::pair<RecoTrack*, float> mcRecoTrackAndWeight =
    prRecoTrack.getRelatedToWithWeight<RecoTrack>(getMCTracksStoreArrayName());
  const RecoTrack* mcRecoTrack = mcRecoTrackAndWeight.first;
  if (mcRecoTrack) {
    purity = mcRecoTrackAndWeight.second;
  } else {
    purity = NAN;
  }
  return mcRecoTrack;
}

const RecoTrack*
TrackMatchLookUp::getRelatedPRRecoTrack(const RecoTrack& mcRecoTrack, float& efficiency) const
{
  assert(isMCRecoTrack(mcRecoTrack));
  std::pair<RecoTrack*, float> prRecoTrackAndWeight =
    mcRecoTrack.getRelatedToWithWeight<RecoTrack>(getPRTracksStoreArrayName());
  const RecoTrack* prRecoTrack = prRecoTrackAndWeight.first;
  if (prRecoTrack) {
    efficiency = prRecoTrackAndWeight.second;
  } else {
    efficiency = NAN;
  }
  return prRecoTrack;
}

TrackMatchLookUp::MCToPRMatchInfo
TrackMatchLookUp::extractMCToPRMatchInfo(const RecoTrack& mcRecoTrack,
                                         const RecoTrack* prRecoTrack,
                                         const float& efficiency) const
{
  if (not prRecoTrack) return MCToPRMatchInfo::c_Missing;
  if (std::isnan(efficiency)) return MCToPRMatchInfo::c_Invalid;
  assert(isMCRecoTrack(mcRecoTrack));
  assert(isPRRecoTrack(*prRecoTrack));

  const RecoTrack* roundTripMCRecoTrack =
    prRecoTrack->getRelatedTo<RecoTrack>(m_mcTracksStoreArrayName);

  if (roundTripMCRecoTrack == &mcRecoTrack) {
    return MCToPRMatchInfo::c_Matched;
  } else {
    return MCToPRMatchInfo::c_Merged;
  }
}

TrackMatchLookUp::PRToMCMatchInfo
TrackMatchLookUp::extractPRToMCMatchInfo(const RecoTrack& prRecoTrack,
                                         const RecoTrack* mcRecoTrack,
                                         const float& purity __attribute__((unused))) const
{
  assert(isPRRecoTrack(prRecoTrack));
  const RecoTrack::MatchingStatus matchingStatus = prRecoTrack.getMatchingStatus();

  if (matchingStatus == RecoTrack::MatchingStatus::c_ghost) {
    return PRToMCMatchInfo::c_Ghost;
  } else if (matchingStatus == RecoTrack::MatchingStatus::c_background) {
    return PRToMCMatchInfo::c_Background;
  } else if (matchingStatus == RecoTrack::MatchingStatus::c_clone) {
    if (not mcRecoTrack) B2WARNING("Clone with no related Monte Carlo RecoTrack");
    return PRToMCMatchInfo::c_Clone;
  } else if (matchingStatus == RecoTrack::MatchingStatus::c_matched) {
    if (not mcRecoTrack) B2WARNING("Match with no related Monte Carlo RecoTrack");
    return PRToMCMatchInfo::c_Matched;
  } else if (matchingStatus == RecoTrack::MatchingStatus::c_undefined) {
    return PRToMCMatchInfo::c_Invalid;
  }
  return PRToMCMatchInfo::c_Invalid;
}

const MCParticle* TrackMatchLookUp::getRelatedMCParticle(const RecoTrack& recoTrack) const
{
  return recoTrack.getRelatedTo<MCParticle>();
}

const RecoTrack* TrackMatchLookUp::getRelatedMCRecoTrack(const RecoTrack& prRecoTrack) const
{
  assert(isPRRecoTrack(prRecoTrack));
  return prRecoTrack.getRelatedTo<RecoTrack>(getMCTracksStoreArrayName());
}

const TrackFitResult* TrackMatchLookUp::getRelatedTrackFitResult(const RecoTrack& prRecoTrack,
    Const::ChargedStable chargedStable) const
{
  assert(isPRRecoTrack(prRecoTrack));
  Belle2::Track* b2track = prRecoTrack.getRelatedFrom<Belle2::Track>();
  if (b2track) {
    // Query the Belle2::Track for the selected fit hypothesis
    return b2track->getTrackFitResult(chargedStable);
  } else {
    return nullptr;
  }
}

const RecoTrack* TrackMatchLookUp::getRelatedPRRecoTrack(const RecoTrack& mcRecoTrack) const
{
  assert(isMCRecoTrack(mcRecoTrack));
  return mcRecoTrack.getRelatedTo<RecoTrack>(getPRTracksStoreArrayName());
}

float TrackMatchLookUp::getRelatedPurity(const RecoTrack& prRecoTrack) const
{
  assert(isPRRecoTrack(prRecoTrack));
  float purity = NAN;
  getRelatedMCRecoTrack(prRecoTrack, purity);
  return std::fabs(purity);
}

float TrackMatchLookUp::getRelatedEfficiency(const RecoTrack& mcRecoTrack) const
{
  assert(isMCRecoTrack(mcRecoTrack));
  float efficiency = NAN;
  getRelatedPRRecoTrack(mcRecoTrack, efficiency);
  return std::fabs(efficiency);
}

const RecoTrack* TrackMatchLookUp::getMatchedMCRecoTrack(const RecoTrack& prRecoTrack) const
{
  assert(isPRRecoTrack(prRecoTrack));

  float purity = NAN;
  const RecoTrack* mcRecoTrack = getRelatedMCRecoTrack(prRecoTrack, purity);

  if (extractPRToMCMatchInfo(prRecoTrack, mcRecoTrack, purity) == PRToMCMatchInfo::c_Matched) {
    return mcRecoTrack;
  } else {
    return nullptr;
  }
}

const RecoTrack* TrackMatchLookUp::getMatchedPRRecoTrack(const RecoTrack& mcRecoTrack) const
{
  assert(isMCRecoTrack(mcRecoTrack));

  float efficiency = NAN;
  const RecoTrack* prRecoTrack = getRelatedPRRecoTrack(mcRecoTrack, efficiency);

  if (extractMCToPRMatchInfo(mcRecoTrack, prRecoTrack, efficiency) == MCToPRMatchInfo::c_Matched) {
    return prRecoTrack;
  } else {
    return nullptr;
  }
}

float TrackMatchLookUp::getMatchedPurity(const RecoTrack& recoTrack) const
{
  if (isMCRecoTrack(recoTrack)) {
    const RecoTrack& mcRecoTrack = recoTrack;
    const RecoTrack* prRecoTrack = getMatchedPRRecoTrack(mcRecoTrack);
    if (prRecoTrack) {
      return getRelatedPurity(*prRecoTrack);
    } else {
      return NAN;
    }

  } else {
    const RecoTrack& prRecoTrack = recoTrack;
    const RecoTrack* mcRecoTrack = getMatchedMCRecoTrack(prRecoTrack);
    if (mcRecoTrack) {
      return getRelatedPurity(prRecoTrack);
    } else {
      return NAN;
    }
  }
}

float TrackMatchLookUp::getMatchedEfficiency(const RecoTrack& recoTrack) const
{
  if (isPRRecoTrack(recoTrack)) {
    const RecoTrack& prRecoTrack = recoTrack;
    const RecoTrack* mcRecoTrack = getMatchedMCRecoTrack(prRecoTrack);
    if (mcRecoTrack) {
      return getRelatedEfficiency(*mcRecoTrack);
    } else {
      return NAN;
    }

  } else {
    const RecoTrack& mcRecoTrack = recoTrack;
    const RecoTrack* prRecoTrack = getMatchedPRRecoTrack(mcRecoTrack);
    if (prRecoTrack) {
      return getRelatedEfficiency(mcRecoTrack);
    } else {
      return NAN;
    }
  }
}
