/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFinderOutputCombiner/StereoSegmentTrackMatcherModule.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(StereoSegmentTrackMatcher);

StereoSegmentTrackMatcherModule::StereoSegmentTrackMatcherModule() : TrackFinderCDCFromSegmentsModule(), m_hitSegmentLookUp()
{
  setDescription("Matches the found stereo segments from the local track finder to the found tracks from the legendre track finder after stereo assignment.\n"
                 "It uses hit based information - that means it matches a segment and a track if they both share hits and if the matching combination is unique.");
}

void StereoSegmentTrackMatcherModule::fillHitLookUp(std::vector<CDCRecoSegment2D>& segments)
{
  m_hitSegmentLookUp.clear();
  for (const CDCRecoSegment2D& segment : segments) {
    if (segment.getStereoType() == AXIAL or segment.getAutomatonCell().hasTakenFlag())
      continue;
    for (const CDCRecoHit2D& recoHit : segment) {
      const CDCHit* cdcHit = recoHit.getWireHit().getHit();
      m_hitSegmentLookUp.insert(std::make_pair(cdcHit, &segment));
    }
  }
}

const CDCRecoSegment2D* StereoSegmentTrackMatcherModule::findMatchingSegment(const CDCRecoHit3D& recoHit)
{
  const CDCHit* cdcHit = recoHit.getWireHit().getHit();
  auto foundElement = m_hitSegmentLookUp.find(cdcHit);
  if (foundElement == m_hitSegmentLookUp.end()) {
    return nullptr;
  } else {
    return foundElement->second;
  }
}

bool segmentMatchesToTrack(const CDCRecoSegment2D* segment, const CDCTrack& track)
{
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  double z0 = track.getStartTrajectory3D().getTrajectorySZ().getStartZ();

  double slopeMeanTrack = 0;
  double slopeMeanSegment = 0;
  unsigned int numberTrackHits = 0;

  for (const CDCRecoHit3D& recoHit : track) {
    if (recoHit.getStereoType() != AXIAL) {
      slopeMeanTrack += recoHit.calculateZSlopeWithZ0(z0);
      numberTrackHits += 1;
    }
  }

  for (const CDCRecoHit2D& recoHit2D : *segment) {
    const CDCRecoHit3D& recoHit3D = CDCRecoHit3D::reconstruct(recoHit2D, trajectory2D);
    slopeMeanSegment += recoHit3D.calculateZSlopeWithZ0(z0);
  }

  slopeMeanSegment /= segment->size();
  slopeMeanTrack /= numberTrackHits;

  //B2INFO(slopeMeanTrack << " segment: " << slopeMeanSegment);
  //return fabs((slopeMeanTrack - slopeMeanSegment) / slopeMeanTrack) < 0.1;
  return true;
}

void StereoSegmentTrackMatcherModule::fillTrackLookUp(std::vector<CDCRecoSegment2D>& segments, std::vector<CDCTrack>& tracks)
{
  // prepare lookup
  m_segmentTrackLookUp.clear();
  for (const CDCRecoSegment2D& segment : segments) {
    m_segmentTrackLookUp.insert(std::make_pair(&segment, std::set<TrackFindingCDC::CDCTrack*>()));
  }

  for (CDCTrack& track : tracks) {
    for (const CDCRecoHit3D& recoHit : track) {
      const CDCRecoSegment2D* matchingSegment = findMatchingSegment(recoHit);
      if (matchingSegment != nullptr and segmentMatchesToTrack(matchingSegment, track)) {
        m_segmentTrackLookUp[matchingSegment].insert(&track);
      }
    }
  }
}

void StereoSegmentTrackMatcherModule::generate(std::vector<CDCRecoSegment2D>& segments, std::vector<CDCTrack>& tracks)
{
  fillHitLookUp(segments);
  fillTrackLookUp(segments, tracks);

  for (auto& segmentTrackCombination : m_segmentTrackLookUp) {
    const CDCRecoSegment2D* segment = segmentTrackCombination.first;
    const std::set<TrackFindingCDC::CDCTrack*>& matchingTracks = segmentTrackCombination.second;
    if (matchingTracks.size() == 1) {
      CDCTrack* track = *(matchingTracks.begin());
      segment->getAutomatonCell().setTakenFlag();
      const CDCTrajectory2D& trajectory2D = track->getStartTrajectory3D().getTrajectory2D();
      for (const CDCRecoHit2D& recoHit2D : *segment) {
        if (not recoHit2D.getWireHit().getAutomatonCell().hasTakenFlag()) {
          CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(recoHit2D, trajectory2D);
          recoHit3D.getWireHit().getAutomatonCell().setTakenFlag();
          track->push_back(recoHit3D);
        }
      }
    }
  }

  for (CDCTrack& track : tracks) {
    // Refit?
    track.sort();
  }
}
