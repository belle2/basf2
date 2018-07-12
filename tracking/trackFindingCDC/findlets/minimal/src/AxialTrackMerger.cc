/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun, Oliver Frost                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/AxialTrackMerger.h>

#include <tracking/trackFindingCDC/processing/AxialTrackUtil.h>

#include <tracking/trackFindingCDC/fitting/CDCKarimakiFitter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

#include <tracking/trackFindingCDC/numerics/WeightComperator.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string AxialTrackMerger::getDescription()
{
  return "Merges axial tracks found in the legendre search";
}

void AxialTrackMerger::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "minFitProb"),
                                m_param_minFitProb,
                                "Minimal fit probability of the common fit "
                                "of two tracks to be eligible for merging",
                                m_param_minFitProb);
}

void AxialTrackMerger::apply(std::vector<CDCTrack>& axialTracks,
                             const std::vector<const CDCWireHit*>& allAxialWireHits)
{
  // Check quality of the track basing on holes on the trajectory;
  // if holes exist then track is splitted
  for (CDCTrack& track : axialTracks) {
    if (track.size() < 5) continue;
    AxialTrackUtil::removeHitsAfterSuperLayerBreak(track);
    AxialTrackUtil::normalizeTrack(track);
  }

  // Update tracks before storing to DataStore
  for (CDCTrack& track : axialTracks) {
    AxialTrackUtil::normalizeTrack(track);
  }

  // Remove bad tracks
  AxialTrackUtil::deleteShortTracks(axialTracks);
  AxialTrackUtil::deleteTracksWithLowFitProbability(axialTracks);

  // Perform tracks merging
  this->doTracksMerging(axialTracks, allAxialWireHits);

  // Remove the consumed, now empty tracks.
  AxialTrackUtil::deleteShortTracks(axialTracks, 0);
}

void AxialTrackMerger::doTracksMerging(std::vector<CDCTrack>& axialTracks,
                                       const std::vector<const CDCWireHit*>& allAxialWireHits)
{
  // Search for best matches - cannot use range for here :(.
  for (auto itTrack = axialTracks.begin(); itTrack !=  axialTracks.end(); ++itTrack) {
    CDCTrack& track = *itTrack;
    auto followingTracks = asRange(std::next(itTrack), axialTracks.end());

    WithWeight<MayBePtr<CDCTrack> > bestTrack = calculateBestTrackToMerge(track, followingTracks);
    double fitProb = bestTrack.getWeight();

    if (bestTrack != nullptr and fitProb > m_param_minFitProb) {
      mergeTracks(track, *bestTrack, allAxialWireHits);
    }
  }

  AxialTrackUtil::deleteShortTracks(axialTracks);
}

template <class ACDCTracks>
WithWeight<MayBePtr<CDCTrack> > AxialTrackMerger::calculateBestTrackToMerge(CDCTrack& track, ACDCTracks& tracks)
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

double AxialTrackMerger::doTracksFitTogether(CDCTrack& track1, CDCTrack& track2)
{
  // First check whether most of the hits from the tracks lie in the backward direction
  // even if though track is not curling -> tracks should not be merged
  const CDCTrajectory3D& trajectory3D1 = track1.getStartTrajectory3D();
  const CDCTrajectory3D& trajectory3D2 = track2.getStartTrajectory3D();

  const Vector2D& phi0Vec1 = trajectory3D1.getFlightDirection3DAtSupport().xy();
  const Vector2D& phi0Vec2 = trajectory3D2.getFlightDirection3DAtSupport().xy();

  int fbVote12 = 0;
  int fbVote21 = 0;

  for (const CDCRecoHit3D& recoHit3D : track1) {
    EForwardBackward fbInfo = phi0Vec2.isForwardOrBackwardOf(recoHit3D.getRecoPos2D());
    if (not isValid(fbInfo)) continue;
    fbVote12 += fbInfo;
  }

  for (const CDCRecoHit3D& recoHit3D : track2) {
    EForwardBackward fbInfo = phi0Vec1.isForwardOrBackwardOf(recoHit3D.getRecoPos2D());
    if (not isValid(fbInfo)) continue;
    fbVote21 += fbInfo;
  }

  if (not trajectory3D1.isCurler() and fbVote12 < 0) return NAN;
  if (not trajectory3D2.isCurler() and fbVote21 < 0) return NAN;

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

  // Dismiss this possibility if the hit list size after all the removing of hits is even smaller
  // than the two lists before or if the list is too small
  if (combinedWireHits.size() <= std::max(track1.size(), track2.size())
      or combinedWireHits.size() < 15) {
    return NAN;
  }

  return commonTrajectory2D.getPValue();
}

void AxialTrackMerger::removeStrangeHits(double factor,
                                         std::vector<const CDCWireHit*>& wireHits,
                                         CDCTrajectory2D& trajectory2D)
{
  auto farFromTrajectory = [&trajectory2D, &factor](const CDCWireHit * wireHit) {
    Vector2D pos2D = wireHit->getRefPos2D();
    double driftLength = wireHit->getRefDriftLength();
    double dist = std::fabs(trajectory2D.getDist2D(pos2D)) - driftLength;
    return std::fabs(dist) > driftLength * factor;
  };
  erase_remove_if(wireHits, farFromTrajectory);
}

void AxialTrackMerger::mergeTracks(CDCTrack& track1,
                                   CDCTrack& track2,
                                   const std::vector<const CDCWireHit*>& allAxialWireHits)
{
  if (&track1 == &track2) return;

  CDCTrajectory2D trajectory2D = track1.getStartTrajectory3D().getTrajectory2D();
  for (const CDCRecoHit3D& orgRecoHit3D : track2) {
    CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(orgRecoHit3D.getRLWireHit(), trajectory2D);
    track1.push_back(std::move(recoHit3D));
  }
  track2.clear();

  AxialTrackUtil::normalizeTrack(track1);

  track2 = CDCTrack(AxialTrackUtil::splitBack2BackTrack(track1));

  AxialTrackUtil::normalizeTrack(track1);

  for (CDCRecoHit3D& recoHit3D : track2) {
    recoHit3D.setRecoPos3D({recoHit3D.getRefPos2D(), 0});
    recoHit3D.setRLInfo(ERightLeft::c_Unknown);
  }

  AxialTrackUtil::normalizeTrack(track2);
  bool success = AxialTrackUtil::postprocessTrack(track2, allAxialWireHits);
  if (not success) {
    for (const CDCRecoHit3D& recoHit3D : track2) {
      recoHit3D.getWireHit()->setTakenFlag(false);
    }
    track2.clear();
  }
}
