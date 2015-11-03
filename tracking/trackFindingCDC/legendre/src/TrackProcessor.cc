/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/TrackProcessor.h>

#include <tracking/trackFindingCDC/quality/TrackQualityTools.h>
#include <tracking/trackFindingCDC/creators/TrackCreator.h>
#include <tracking/trackFindingCDC/legendre/HitProcessor.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>

#include <tracking/trackFindingCDC/eventdata/hits/ConformalCDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


void TrackProcessor::addCandidateWithHits(std::vector<const CDCWireHit*>& hits,
                                          const std::vector<ConformalCDCWireHit>& conformalCDCWireHitList,
                                          CDCTrackList& cdcTrackList)
{
  if (hits.size() == 0) return;

  CDCObservations2D observations;
  for (const CDCWireHit* item : hits) {
    observations.append(*item);
  }

  CDCTrack& track = cdcTrackList.createEmptyTrack();

  const CDCRiemannFitter& fitter = CDCRiemannFitter::getFitter();
  const CDCTrajectory2D& trackTrajectory2D = fitter.fit(observations);

  for (const CDCWireHit* item : hits) {
    if (item->getAutomatonCell().hasTakenFlag() || item->getAutomatonCell().hasMaskedFlag()) continue;

    const CDCRecoHit3D& cdcRecoHit3D = CDCRecoHit3D::reconstructNearest(*item, trackTrajectory2D);
    track.push_back(std::move(cdcRecoHit3D));
    cdcRecoHit3D.getWireHit().getAutomatonCell().setTakenFlag(true);
  }

  CDCTrajectory3D trajectory3D(trackTrajectory2D, CDCTrajectorySZ::basicAssumption());
  track.setStartTrajectory3D(trajectory3D);

  postprocessTrack(track, conformalCDCWireHitList, cdcTrackList);
}

void TrackProcessor::postprocessTrack(CDCTrack& track, const std::vector<ConformalCDCWireHit>& conformalCDCWireHitList,
                                      CDCTrackList& cdcTrackList)
{
  TrackQualityTools::normalizeTrack(track);

  HitProcessor::splitBack2BackTrack(track);
  if (not checkTrackQuality(track)) {
    for (const CDCRecoHit3D& hit : track) {
      hit.getWireHit().getAutomatonCell().setMaskedFlag(true);
      hit.getWireHit().getAutomatonCell().setTakenFlag(false);
    }
    cdcTrackList.getCDCTracks().remove(track);
    return;
  }

  TrackQualityTools::normalizeTrack(track);

  deleteHitsFarAwayFromTrajectory(track);
  TrackQualityTools::normalizeTrack(track);

  if (not checkTrackQuality(track)) {
    for (const CDCRecoHit3D& hit : track) {
      hit.getWireHit().getAutomatonCell().setMaskedFlag(true);
      hit.getWireHit().getAutomatonCell().setTakenFlag(false);
    }
    cdcTrackList.getCDCTracks().remove(track);
    return;
  }

  TrackQualityTools::normalizeTrack(track);

  assignNewHitsToTrack(track, conformalCDCWireHitList);

  //  B2INFO("split");
  HitProcessor::splitBack2BackTrack(track);
  if (not checkTrackQuality(track)) {
    for (const CDCRecoHit3D& hit : track) {
      hit.getWireHit().getAutomatonCell().setMaskedFlag(true);
      hit.getWireHit().getAutomatonCell().setTakenFlag(false);
    }
    cdcTrackList.getCDCTracks().remove(track);
    return;
  }

  TrackQualityTools::normalizeTrack(track);

  deleteHitsFarAwayFromTrajectory(track);
  TrackQualityTools::normalizeTrack(track);

  if (not checkTrackQuality(track)) {
    for (const CDCRecoHit3D& hit : track) {
      hit.getWireHit().getAutomatonCell().setMaskedFlag(true);
      hit.getWireHit().getAutomatonCell().setTakenFlag(false);
    }
    cdcTrackList.getCDCTracks().remove(track);
    return;
  }

  TrackQualityTools::normalizeTrack(track);

  assignNewHitsToTrack(track, conformalCDCWireHitList);

  TrackQualityTools::normalizeTrack(track);

  for (const CDCRecoHit3D& hit : track) {
    hit.getWireHit().getAutomatonCell().setMaskedFlag(false);
    hit.getWireHit().getAutomatonCell().setTakenFlag(true);
  }

}

bool TrackProcessor::checkTrackQuality(const CDCTrack& track)
{
  return track.size() >= 5;
}

void TrackProcessor::deleteHitsFarAwayFromTrajectory(CDCTrack& track, double maximum_distance)
{
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  for (CDCRecoHit3D& recoHit : track) {
    const Vector2D& recoPos2D = recoHit.getRecoPos2D();
    if (fabs(trajectory2D.getDist2D(recoPos2D)) > maximum_distance)
      recoHit->getWireHit().getAutomatonCell().setMaskedFlag(true);
  }

  HitProcessor::deleteAllMarkedHits(track);
}

void TrackProcessor::assignNewHits(const std::vector<ConformalCDCWireHit>& conformalCDCWireHitList, CDCTrackList& cdcTrackList)
{
  cdcTrackList.getCDCTracks().erase(std::remove_if(cdcTrackList.getCDCTracks().begin(), cdcTrackList.getCDCTracks().end(),
  [](const CDCTrack & track) {
    return track.size() == 0;
  }) , cdcTrackList.getCDCTracks().end());


//  for (CDCTrack& track : m_cdcTracks) {
  cdcTrackList.doForAllTracks([&](CDCTrack & track) {

    if (track.size() < 4) return;

    assignNewHitsToTrack(track, conformalCDCWireHitList);

    std::vector<const CDCWireHit*> removedHits = HitProcessor::splitBack2BackTrack(track);

    addCandidateWithHits(removedHits, conformalCDCWireHitList, cdcTrackList);

    //  B2INFO("update");
    TrackQualityTools::normalizeTrack(track);

    //  B2INFO("delete");
    deleteHitsFarAwayFromTrajectory(track);

    TrackQualityTools::normalizeTrack(track);
  });

  HitProcessor::reassignHitsFromOtherTracks(cdcTrackList);

  cdcTrackList.doForAllTracks([](CDCTrack & cand) {
    cand.forwardTakenFlag();
  });
}

void TrackProcessor::assignNewHitsToTrack(CDCTrack& track, const std::vector<ConformalCDCWireHit>& conformalCDCWireHitList,
                                          double minimal_distance_to_track)
{
  if (track.size() < 10) return;
  HitProcessor::unmaskHitsInTrack(track);

  const CDCTrajectory2D& trackTrajectory2D = track.getStartTrajectory3D().getTrajectory2D();

  for (const ConformalCDCWireHit& hit : conformalCDCWireHitList) {
    if (hit.getUsedFlag() or hit.getMaskedFlag()) {
      continue;
    }

    const CDCRecoHit3D& cdcRecoHit3D = CDCRecoHit3D::reconstructNearest(*(hit.getCDCWireHit()), trackTrajectory2D);
    const Vector2D& recoPos2D = cdcRecoHit3D.getRecoPos2D();

    if (fabs(trackTrajectory2D.getDist2D(recoPos2D)) < minimal_distance_to_track) {
      track.push_back(std::move(cdcRecoHit3D));
      cdcRecoHit3D.getWireHit().getAutomatonCell().setTakenFlag();
    }
  }

  TrackQualityTools::normalizeTrack(track);
}

void TrackProcessor::deleteTracksWithLowFitProbability(CDCTrackList& cdcTrackList, double minimal_probability_for_good_fit)
{
  cdcTrackList.doForAllTracks([&](CDCTrack & track) {
    const CDCTrajectory2D& fittedTrajectory = HitProcessor::fitWhithoutRecoPos(track);
    const double chi2 = fittedTrajectory.getChi2();
    const int dof = track.size() - 4;

    if (TMath::Prob(chi2, dof) < minimal_probability_for_good_fit) {
      // Remove all hits = delete the track.
      track.clear();
    }
  });
}


bool TrackProcessor::isChi2InQuantiles(CDCTrack& track, double lower_quantile, double upper_quantile)
{
  const CDCTrajectory2D& trackTrajectory2D = track.getStartTrajectory3D().getTrajectory2D();

  const double minChi2 = calculateChi2ForQuantile(lower_quantile, trackTrajectory2D.getNDF());
  const double maxChi2 = calculateChi2ForQuantile(upper_quantile, trackTrajectory2D.getNDF());
  const double chi2 = trackTrajectory2D.getChi2();

  return ((chi2 >= minChi2) and (chi2 <= maxChi2));
}

double TrackProcessor::calculateChi2ForQuantile(double alpha, double n)
{
  double d;
  if (alpha > 0.5) {
    d = 2.0637 * pow(log(1. / (1. - alpha)) - 0.16, 0.4274) - 1.5774;
  } else {
    d = -2.0637 * pow(log(1. / alpha) - 0.16, 0.4274) + 1.5774;
  }

  const double A = d * pow(2., 0.5);
  const double B = 2.*(d * d - 1.) / 3.;
  const double C = d * (d * d - 7) / (9. * pow(2., 0.5));
  const double D = (6.*d * d * d * d + 14 * d * d - 32) / 405.;
  const double E = d * (9 * d * d * d * d + 256 * d * d - 433) / (4860. * pow(2., 0.5));

  return n + A * pow(n, 0.5) + B + C / pow(n, 0.5) + D / n + E / (n * pow(n, 0.5));
}

