/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/TrackFactory.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <cdc/dataobjects/CDCHit.h>
#include <tracking/trackFindingCDC/legendre/HitProcessor.h>
#include <tracking/trackFindingCDC/legendre/TrackFitter.h>
#include <tracking/trackFindingCDC/legendre/ConformalExtension.h>

#include <genfit/TrackCand.h>
#include <framework/gearbox/Const.h>
#include "../include/TrackHitsProcessor.h"

#include "TCanvas.h"
#include "TH1F.h"

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

CDCTrack& TrackFactory::createCDCTrackCandidate(std::vector<QuadTreeHitWrapper*>& qtHits)
{
  m_cdcTracks.emplace_back();
  CDCTrack& newTrackCandidate = m_cdcTracks.back();

  if (qtHits.size() == 0) return newTrackCandidate;

  CDCObservations2D observations2DLegendre;
  for (const QuadTreeHitWrapper* item : qtHits) {
    observations2DLegendre.append(*item->getCDCWireHit());
  }

  CDCTrajectory2D trackTrajectory2D ;
  m_trackFitter.update(trackTrajectory2D, observations2DLegendre);

  for (QuadTreeHitWrapper* trackHit : qtHits) {
    if (trackHit->getUsedFlag() || trackHit->getMaskedFlag()) continue;

    const CDCRecoHit3D& cdcRecoHit3D  = HitProcessor::createRecoHit3D(trackTrajectory2D, trackHit);
    newTrackCandidate.push_back(std::move(cdcRecoHit3D));
    cdcRecoHit3D.getWireHit().getAutomatonCell().setTakenFlag(true);
  }

  CDCTrajectory3D trajectory3D(trackTrajectory2D, CDCTrajectorySZ::basicAssumption());
  newTrackCandidate.setStartTrajectory3D(trajectory3D);

  return newTrackCandidate;
}



CDCTrack& TrackFactory::createCDCTrackCandidate(std::vector<const CDCWireHit*>& hits)
{
  //  B2INFO("create");
  m_cdcTracks.emplace_front();
  CDCTrack& newTrackCandidate = m_cdcTracks.front();


  CDCObservations2D observations;
  for (const CDCWireHit* item : hits) {
    observations.append(*item);
  }

  CDCTrajectory2D trackTrajectory2D ;
  m_trackFitter.update(trackTrajectory2D, observations);


  for (const CDCWireHit* item : hits) {
    if (item->getAutomatonCell().hasTakenFlag() || item->getAutomatonCell().hasMaskedFlag()) continue;

    const CDCRecoHit3D& cdcRecoHit3D  = HitProcessor::createRecoHit3D(trackTrajectory2D, item);
    newTrackCandidate.push_back(std::move(cdcRecoHit3D));
    cdcRecoHit3D.getWireHit().getAutomatonCell().setTakenFlag(true);
  }

  CDCTrajectory3D trajectory3D(trackTrajectory2D, CDCTrajectorySZ::basicAssumption());
  newTrackCandidate.setStartTrajectory3D(trajectory3D);

  return newTrackCandidate;

}


void TrackFactory::createCDCTracks(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
{
//  tracks.clear();

//  std::copy(m_tracksVector.begin(), m_tracksVector.end(), tracks.begin());

  for (CDCTrack& track : m_cdcTracks) {
    if (track.size() > 5) tracks.push_back(std::move(track));
  }
}

void TrackFactory::deleteTrack(CDCTrack& track)
{

}

CDCTrajectory2D TrackFactory::fit(CDCTrack& track)
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
