/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun, Dmitrii Neverov               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/quadtree/BaseCandidateReceiver.h>
#include <tracking/trackFindingCDC/processing/LowHitsAxialTrackUtil.h>

using namespace Belle2;
using namespace TrackFindingCDC;

BaseCandidateReceiver::BaseCandidateReceiver(std::vector<const CDCWireHit*> allAxialWireHits)
  : m_allAxialWireHits(std::move(allAxialWireHits))
{
}

BaseCandidateReceiver::~BaseCandidateReceiver()
{
}

void BaseCandidateReceiver::operator()(const std::vector<const CDCWireHit*>& inputWireHits,
                                       void* qt __attribute__((unused)))
{
  // Unset the taken flag
  // NOTE after QuadTreeProcessor finds a leaf, it marks all items as "taken"
  for (const CDCWireHit* wireHit : inputWireHits) {
    (*wireHit)->setTakenFlag(false);
  }
  LowHitsAxialTrackUtil::addCandidateFromHits(inputWireHits,
                                              m_allAxialWireHits,
                                              m_tracks,
                                              true, // fromOrigin
                                              false, // straight
                                              false); // withPostprocessing
}

const std::vector<CDCTrack>& BaseCandidateReceiver::getTracks() const
{
  return m_tracks;
}
