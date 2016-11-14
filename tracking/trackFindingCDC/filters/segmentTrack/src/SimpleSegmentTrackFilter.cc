/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentTrack/SimpleSegmentTrackFilter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

Weight SimpleSegmentTrackFilter::operator()(const std::pair<const CDCRecoSegment2D*, const CDCTrack*>& testPair)
{
  Super::operator()(testPair);

  const std::map<std::string, Float_t>& varSet = Super::getVarSet().getNamedValues();
  bool is_stereo = testPair.first->getStereoKind() != EStereoKind::c_Axial;
  double hit_distance = varSet.at("maxmimum_hit_distance_front");

  if (is_stereo) {
    if (hit_distance > 10) {
      B2DEBUG(120, "Hits too far away: " << hit_distance);
      return NAN;
    }
  } else {
    if (hit_distance > 2) {
      B2DEBUG(120, "Hits too far away: " << hit_distance);
      return NAN;
    }
  }

  bool out_of_CDC = varSet.at("out_of_CDC") == 1.0 ? true : false;

  if (is_stereo and out_of_CDC) {
    return NAN;
  }

  unsigned int hits_in_same_region = static_cast<unsigned int>(varSet.at("hits_in_same_region"));
  unsigned int segment_size = static_cast<unsigned int>(varSet.at("segment_size"));

  if (hits_in_same_region > 5) {
    B2DEBUG(110, "Too many hits in the same region: " << hits_in_same_region);
    return NAN;
  } else {
    B2DEBUG(110, "Hits in the region: " << hits_in_same_region << " while hits in segment: " << segment_size);
  }

  // Means: yes
  return 1.0;
}
