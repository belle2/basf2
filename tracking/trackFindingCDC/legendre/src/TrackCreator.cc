/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/TrackCreator.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCKarimakiFitter.h>
#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTree.h>
#include <tracking/trackFindingCDC/legendre/QuadTreeHitWrapper.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <cdc/dataobjects/CDCHit.h>
#include <tracking/trackFindingCDC/legendre/HitFactory.h>

#include <genfit/TrackCand.h>
#include <framework/gearbox/Const.h>
#include "../include/TrackHitsProcessor.h"

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

void TrackCreator::createCDCTrackCandidate(std::vector<QuadTreeHitWrapper*>& qtHits, CDCTrack& track)
{
  CDCObservations2D observations2DLegendre;
  for (const QuadTreeHitWrapper* item : qtHits) {
    observations2DLegendre.append(*item->getCDCWireHit());
  }

  CDCTrajectory2D trackTrajectory2D ;
  m_trackFitter.update(trackTrajectory2D, observations2DLegendre);

  for (QuadTreeHitWrapper* trackHit : qtHits) {
    if (trackHit->getUsedFlag() || trackHit->getMaskedFlag()) continue;

    const CDCRecoHit3D& cdcRecoHit3D  = HitFactory::createRecoHit3D(trackTrajectory2D, trackHit);
    track.push_back(std::move(cdcRecoHit3D));
    cdcRecoHit3D.getWireHit().getAutomatonCell().setTakenFlag(true);
  }

  CDCTrajectory3D trajectory3D(trackTrajectory2D, CDCTrajectorySZ::basicAssumption());
  track.setStartTrajectory3D(trajectory3D);
}



void TrackCreator::createCDCTrackCandidate(std::vector<const CDCWireHit*>& hits, CDCTrack& track)
{
  CDCObservations2D observations;
  for (const CDCWireHit* item : hits) {
    observations.append(*item);
  }

  CDCTrajectory2D trackTrajectory2D ;
  m_trackFitter.update(trackTrajectory2D, observations);


  for (const CDCWireHit* item : hits) {
    if (item->getAutomatonCell().hasTakenFlag() || item->getAutomatonCell().hasMaskedFlag()) continue;

    const CDCRecoHit3D& cdcRecoHit3D  = HitFactory::createRecoHit3D(trackTrajectory2D, item);
    track.push_back(std::move(cdcRecoHit3D));
    cdcRecoHit3D.getWireHit().getAutomatonCell().setTakenFlag(true);
  }

  CDCTrajectory3D trajectory3D(trackTrajectory2D, CDCTrajectorySZ::basicAssumption());
  track.setStartTrajectory3D(trajectory3D);
}


void TrackCreator::createCDCTracks(std::vector<CDCTrack>& tracksToStore, std::list<CDCTrack>& tracksFromFinder)
{
  tracksToStore.reserve(tracksToStore.size() + tracksFromFinder.size());

  for (CDCTrack& track : tracksFromFinder) {
    if (track.size() > 5) tracksToStore.push_back(std::move(track));
  }
}

CDCTrajectory2D TrackCreator::fit(CDCTrack& track)
{
  bool m_usePosition(true);
//  bool m_useOrientation(false);

  CDCTrajectory2D result;
  CDCObservations2D observations2D;
  observations2D.setUseRecoPos(m_usePosition);
  size_t nAppendedHits = 0;
  for (const CDCRecoHit3D& item : track.items()) {
    nAppendedHits += observations2D.append(item, m_usePosition);
  }

  m_trackFitter.update(result, observations2D);

  return result;
}
