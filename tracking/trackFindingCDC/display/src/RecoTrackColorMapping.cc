/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/display/RecoTrackColorMapping.h>

#include <tracking/mcMatcher/TrackMatchLookUp.h>

#include <tracking/dataobjects/RecoTrack.h>

using namespace Belle2;
using namespace TrackFindingCDC;

const std::string c_bkgRecoTrackColor = "orange";

std::string RecoTrackMatchingStatusColorMap::map(int index __attribute__((unused)),
                                                 const RecoTrack& recoTrack)
{
  using MatchingStatus = RecoTrack::MatchingStatus;
  MatchingStatus matchingStatus = recoTrack.getMatchingStatus();
  switch (matchingStatus) {
    case MatchingStatus::c_matched:
      B2DEBUG(28, "Draw RecoTrack " << index << " as matched");
      return "green";
      break;

    case MatchingStatus::c_background:
      B2DEBUG(28, "Draw RecoTrack " << index << " as background");
      return "purple";
      break;

    case MatchingStatus::c_clone:
      B2DEBUG(28, "Draw RecoTrack " << index << " as clone");
      return "blue";
      break;

    case MatchingStatus::c_ghost:
      B2DEBUG(28, "Draw RecoTrack " << index << " as ghost");
      return "red";
      break;

    default:
      B2WARNING("Undefined matching status");
      return c_bkgRecoTrackColor;
      break;
  }
}

std::string RecoTrackMatchingStatusColorMap::info()
{
  return "Matching status:\n green <-> match\n blue <-> clone\n red <-> ghost\n purple <-> background.";
}

std::string MCRecoTrackMatchingStatusColorMap::map(int index __attribute__((unused)),
                                                   const RecoTrack& recoTrack)
{
  const std::string mcRecoTrackStoreArrayName = "MCRecoTracks";
  TrackMatchLookUp trackMatchLookUp(mcRecoTrackStoreArrayName);
  using MCToPRMatchInfo = TrackMatchLookUp::MCToPRMatchInfo;
  MCToPRMatchInfo mcMatchInfo = trackMatchLookUp.getMCToPRMatchInfo(recoTrack);
  MCParticle* mcParticle = recoTrack.getRelatedTo<MCParticle>();
  bool isPrimary = mcParticle->isPrimaryParticle();
  switch (mcMatchInfo) {
    case MCToPRMatchInfo::c_matched:
      return isPrimary ?  "green" : "limegreen";
      break;

    case MCToPRMatchInfo::c_merged:
      return isPrimary ?  "blue" : "cornflowerblue";
      break;

    case MCToPRMatchInfo::c_missing:
      return isPrimary ?  "darkred" : "crimson";
      break;

    default:
      B2WARNING("Undefined matching status");
      return c_bkgRecoTrackColor;
      break;
  }
}

std::string MCRecoTrackMatchingStatusColorMap::info()
{
  return "Matching status:\n green <-> match\n blue <-> merged\n red <-> missing.";
}
