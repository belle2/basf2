/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
