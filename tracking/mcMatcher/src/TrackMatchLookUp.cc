/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/mcMatcher/TrackMatchLookUp.h>

#include <mdst/dataobjects/Track.h>

#include <framework/datastore/DataStore.h>
#include <framework/logging/Logger.h>

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
  B2ASSERT("This RecoTrack isn't a PR RecoTrack as it is supposed to be.", isPRRecoTrack(prRecoTrack));
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
  B2ASSERT("This RecoTrack isn't a MC RecoTrack as it is supposed to be.", isMCRecoTrack(mcRecoTrack));
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
  if (not prRecoTrack) return MCToPRMatchInfo::c_missing;
  if (std::isnan(efficiency)) return MCToPRMatchInfo::c_undefined;
  B2ASSERT("This RecoTrack isn't a MC RecoTrack as it is supposed to be.", isMCRecoTrack(mcRecoTrack));
  B2ASSERT("This RecoTrack isn't a PR RecoTrack as it is supposed to be.", isPRRecoTrack(*prRecoTrack));

  const RecoTrack* roundTripMCRecoTrack =
    prRecoTrack->getRelatedTo<RecoTrack>(m_mcTracksStoreArrayName);

  const bool wrongCharge = roundTripMCRecoTrack->getChargeSeed() != mcRecoTrack.getChargeSeed();

  if (roundTripMCRecoTrack == &mcRecoTrack) {
    if (wrongCharge) return MCToPRMatchInfo::c_matchedWrongCharge;
    else return MCToPRMatchInfo::c_matched;
  } else {
    if (wrongCharge) return MCToPRMatchInfo::c_mergedWrongCharge;
    else return MCToPRMatchInfo::c_merged;
  }
}

TrackMatchLookUp::PRToMCMatchInfo
TrackMatchLookUp::extractPRToMCMatchInfo(const RecoTrack& prRecoTrack,
                                         const RecoTrack* mcRecoTrack,
                                         const float& purity __attribute__((unused))) const
{
  B2ASSERT("This RecoTrack isn't a PR RecoTrack as it is supposed to be.", isPRRecoTrack(prRecoTrack));
  const RecoTrack::MatchingStatus matchingStatus = prRecoTrack.getMatchingStatus();

  if (matchingStatus == RecoTrack::MatchingStatus::c_ghost) {
    return PRToMCMatchInfo::c_ghost;
  } else if (matchingStatus == RecoTrack::MatchingStatus::c_background) {
    return PRToMCMatchInfo::c_background;
  } else if (matchingStatus == RecoTrack::MatchingStatus::c_clone) {
    if (not mcRecoTrack) B2WARNING("Clone with no related Monte Carlo RecoTrack");
    return PRToMCMatchInfo::c_clone;
  }  else if (matchingStatus == RecoTrack::MatchingStatus::c_cloneWrongCharge) {
    if (not mcRecoTrack) B2WARNING("Clone with no related Monte Carlo RecoTrack");
    return PRToMCMatchInfo::c_cloneWrongCharge;
  } else if (matchingStatus == RecoTrack::MatchingStatus::c_matched) {
    if (not mcRecoTrack) B2WARNING("Match with no related Monte Carlo RecoTrack");
    return PRToMCMatchInfo::c_matched;
  }  else if (matchingStatus == RecoTrack::MatchingStatus::c_matchedWrongCharge) {
    if (not mcRecoTrack) B2WARNING("Match with no related Monte Carlo RecoTrack");
    return PRToMCMatchInfo::c_matchedWrongCharge;
  } else if (matchingStatus == RecoTrack::MatchingStatus::c_undefined) {
    return PRToMCMatchInfo::c_undefined;
  }
  return PRToMCMatchInfo::c_undefined;
}

const MCParticle* TrackMatchLookUp::getRelatedMCParticle(const RecoTrack& recoTrack) const
{
  return recoTrack.getRelatedTo<MCParticle>();
}

const RecoTrack* TrackMatchLookUp::getRelatedMCRecoTrack(const RecoTrack& prRecoTrack) const
{
  B2ASSERT("This RecoTrack isn't a PR RecoTrack as it is supposed to be.", isPRRecoTrack(prRecoTrack));
  return prRecoTrack.getRelatedTo<RecoTrack>(getMCTracksStoreArrayName());
}

const TrackFitResult* TrackMatchLookUp::getRelatedTrackFitResult(const RecoTrack& prRecoTrack,
    Const::ChargedStable chargedStable) const
{
  B2ASSERT("This RecoTrack isn't a PR RecoTrack as it is supposed to be.", isPRRecoTrack(prRecoTrack));
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
  B2ASSERT("This RecoTrack isn't a MC RecoTrack as it is supposed to be.", isMCRecoTrack(mcRecoTrack));
  return mcRecoTrack.getRelatedTo<RecoTrack>(getPRTracksStoreArrayName());
}

float TrackMatchLookUp::getRelatedPurity(const RecoTrack& prRecoTrack) const
{
  B2ASSERT("This RecoTrack isn't a PR RecoTrack as it is supposed to be.", isPRRecoTrack(prRecoTrack));
  float purity = NAN;
  getRelatedMCRecoTrack(prRecoTrack, purity);
  return std::fabs(purity);
}

float TrackMatchLookUp::getRelatedEfficiency(const RecoTrack& mcRecoTrack) const
{
  B2ASSERT("This RecoTrack isn't a MC RecoTrack as it is supposed to be.", isMCRecoTrack(mcRecoTrack));
  float efficiency = NAN;
  getRelatedPRRecoTrack(mcRecoTrack, efficiency);
  return std::fabs(efficiency);
}

const RecoTrack* TrackMatchLookUp::getMatchedMCRecoTrack(const RecoTrack& prRecoTrack) const
{
  B2ASSERT("This RecoTrack isn't a PR RecoTrack as it is supposed to be.", isPRRecoTrack(prRecoTrack));

  float purity = NAN;
  const RecoTrack* mcRecoTrack = getRelatedMCRecoTrack(prRecoTrack, purity);

  if (extractPRToMCMatchInfo(prRecoTrack, mcRecoTrack, purity) == PRToMCMatchInfo::c_matched) {
    return mcRecoTrack;
  } else {
    return nullptr;
  }
}

const RecoTrack* TrackMatchLookUp::getWrongChargeMCRecoTrack(const RecoTrack& prRecoTrack) const
{
  B2ASSERT("This RecoTrack isn't a PR RecoTrack as it is supposed to be.", isPRRecoTrack(prRecoTrack));

  float purity = NAN;
  const RecoTrack* mcRecoTrack = getRelatedMCRecoTrack(prRecoTrack, purity);

  if (extractPRToMCMatchInfo(prRecoTrack, mcRecoTrack, purity) == PRToMCMatchInfo::c_matchedWrongCharge) {
    return mcRecoTrack;
  } else {
    return nullptr;
  }
}

const RecoTrack* TrackMatchLookUp::getMatchedPRRecoTrack(const RecoTrack& mcRecoTrack) const
{
  B2ASSERT("This RecoTrack isn't a MC RecoTrack as it is supposed to be.", isMCRecoTrack(mcRecoTrack));

  float efficiency = NAN;
  const RecoTrack* prRecoTrack = getRelatedPRRecoTrack(mcRecoTrack, efficiency);

  if (extractMCToPRMatchInfo(mcRecoTrack, prRecoTrack, efficiency) == MCToPRMatchInfo::c_matched) {
    return prRecoTrack;
  } else {
    return nullptr;
  }
}

const RecoTrack* TrackMatchLookUp::getWrongChargePRRecoTrack(const RecoTrack& mcRecoTrack) const
{
  B2ASSERT("This RecoTrack isn't a MC RecoTrack as it is supposed to be.", isMCRecoTrack(mcRecoTrack));

  float efficiency = NAN;
  const RecoTrack* prRecoTrack = getRelatedPRRecoTrack(mcRecoTrack, efficiency);

  if (extractMCToPRMatchInfo(mcRecoTrack, prRecoTrack, efficiency) == MCToPRMatchInfo::c_matchedWrongCharge) {
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


bool TrackMatchLookUp::isChargeMatched(const RecoTrack& recoTrack) const
{
  const MCParticle* mcParticle = getRelatedMCParticle(recoTrack);

  if (mcParticle)
    return (mcParticle->getCharge() == recoTrack.getChargeSeed());
  else
    return NAN;

}
