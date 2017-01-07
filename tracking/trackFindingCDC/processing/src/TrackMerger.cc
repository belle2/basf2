/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun, Oliver Frost                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/processing/TrackMerger.h>

#include <tracking/trackFindingCDC/processing/TrackQualityTools.h>
#include <tracking/trackFindingCDC/processing/TrackProcessor.h>
#include <tracking/trackFindingCDC/processing/HitProcessor.h>

#include <tracking/trackFindingCDC/fitting/CDCKarimakiFitter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/utilities/Functional.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void TrackMerger::doTracksMerging(std::list<CDCTrack>& cdcTrackList,
                                  const std::vector<const CDCWireHit*>& allAxialWireHits,
                                  double minimum_probability_to_be_merged)
{
  // Search for best matches - cannot use range for here :(.
  for (std::list<CDCTrack>::iterator it1 = cdcTrackList.begin(); it1 !=  cdcTrackList.end(); ++it1) {
    CDCTrack& track = *it1;
    auto followingTracks = asRange(std::next(it1), cdcTrackList.end());

    WithWeight<MayBePtr<CDCTrack> > bestTrack = calculateBestTrackToMerge(track, followingTracks);
    double fitProb = bestTrack.getWeight();

    if (bestTrack != nullptr and fitProb > minimum_probability_to_be_merged) {
      mergeTracks(track, *bestTrack, allAxialWireHits, cdcTrackList);
    }
  }

  erase_remove_if(cdcTrackList, Size() < 3u);
}

template <class ACDCTracks>
WithWeight<MayBePtr<CDCTrack> > TrackMerger::calculateBestTrackToMerge(CDCTrack& track, ACDCTracks& tracks)
{
  std::vector<WithWeight<CDCTrack*>> weightedTracks;
  for (CDCTrack& track2 : tracks) {
    if (&track == &track2) continue;
    if (track2.size() < 3) continue;

    double fitProb = doTracksFitTogether(track, track2);
    if (std::isnan(fitProb)) continue;

    weightedTracks.emplace_back(&track2, fitProb);
  }

  auto bestMatch = std::max_element(weightedTracks.begin(), weightedTracks.end(), LessWeight());
  if (bestMatch == weightedTracks.end()) return {nullptr, 0};
  else return *bestMatch;
}

double TrackMerger::doTracksFitTogether(CDCTrack& track1, CDCTrack& track2)
{
  // Build common hit list by copying the wire hits into one large list
  // We use the wire hits here as we do not want them to bring
  // their "old" reconstructed position when fitting.
  std::vector<const CDCWireHit*> combinedWireHits;
  for (const CDCRecoHit3D& hit : track1) {
    combinedWireHits.push_back(&(hit.getWireHit()));
  }
  for (const CDCRecoHit3D& hit : track2) {
    combinedWireHits.push_back(&(hit.getWireHit()));
  }

  // Sorting is done via pointer addresses (!!).
  // This is not very stable and also not very meaningful (in terms of ordering in the track),
  // but it does the job for unique.
  // (the ordering is still outwards though since the wire hits are ordered like that in continuous memory)
  std::sort(combinedWireHits.begin(), combinedWireHits.end());
  erase_unique(combinedWireHits);

  // Calculate track parameters
  CDCTrajectory2D commonTrajectory2D;
  const CDCKarimakiFitter& fitter = CDCKarimakiFitter::getNoDriftVarianceFitter();

  // Approach the best fit
  commonTrajectory2D = fitter.fit(combinedWireHits);
  removeStrangeHits(5, combinedWireHits, commonTrajectory2D);
  commonTrajectory2D = fitter.fit(combinedWireHits);
  removeStrangeHits(3, combinedWireHits, commonTrajectory2D);
  commonTrajectory2D = fitter.fit(combinedWireHits);
  removeStrangeHits(1, combinedWireHits, commonTrajectory2D);
  commonTrajectory2D = fitter.fit(combinedWireHits);
  removeStrangeHits(1, combinedWireHits, commonTrajectory2D);
  commonTrajectory2D = fitter.fit(combinedWireHits);

  // TODO: perform B2B tracks check
  // if B2B return 0;

  // Dismiss this possibility if the hit list size after all the removing of hits is even smaller
  // than the two lists before or if the list is too small
  if (combinedWireHits.size() <= std::max(track1.size(), track2.size())
      or combinedWireHits.size() < 15) {
    return NAN;
  }

  return commonTrajectory2D.getPValue();
}

void TrackMerger::removeStrangeHits(double factor, std::vector<const CDCWireHit*>& wireHits, CDCTrajectory2D& trajectory2D)
{
  auto farFromTrajectory = [&trajectory2D, &factor](const CDCWireHit * wireHit) {
    Vector2D pos2D = wireHit->getRefPos2D();
    double driftLength = wireHit->getRefDriftLength();
    double dist = std::fabs(trajectory2D.getDist2D(pos2D)) - driftLength;
    return std::fabs(dist) > driftLength * factor;
  };
  erase_remove_if(wireHits, farFromTrajectory);
}

void TrackMerger::mergeTracks(CDCTrack& track1,
                              CDCTrack& track2,
                              const std::vector<const CDCWireHit*>& allAxialWireHits,
                              std::list<CDCTrack>& cdcTrackList)
{
  if (&track1 == &track2) return;

  CDCTrajectory2D trajectory2D = track1.getStartTrajectory3D().getTrajectory2D();
  for (const CDCRecoHit3D& orgRecoHit3D : track2) {
    CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(orgRecoHit3D.getRLWireHit(), trajectory2D);
    track1.push_back(std::move(recoHit3D));
  }
  track2.clear();

  TrackQualityTools::normalizeTrack(track1);

  std::vector<const CDCWireHit*> splittedHits = HitProcessor::splitBack2BackTrack(track1);

  TrackQualityTools::normalizeTrack(track1);

  TrackProcessor::addCandidateFromHitsWithPostprocessing(splittedHits, allAxialWireHits, cdcTrackList);
}

void TrackMerger::tryToMergeTrackWithOtherTracks(CDCTrack& track,
                                                 std::list<CDCTrack>& cdcTrackList,
                                                 const std::vector<const CDCWireHit*>& allAxialWireHits,
                                                 double minimum_probability_to_be_merged)
{
  bool have_merged_something;
  do {
    have_merged_something = false;
    WithWeight<MayBePtr<CDCTrack> > bestTrack = calculateBestTrackToMerge(track, cdcTrackList);
    double fitProb = bestTrack.getWeight();

    if (bestTrack and fitProb > minimum_probability_to_be_merged) {
      mergeTracks(track, *bestTrack, allAxialWireHits, cdcTrackList);
      have_merged_something = true;
    }

    erase_remove_if(cdcTrackList, Size() < 3u);
  } while (have_merged_something);

  for (CDCTrack& otherTrack : cdcTrackList) {
    TrackQualityTools::normalizeTrack(otherTrack);
  }
}
