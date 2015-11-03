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
#include <tracking/trackFindingCDC/fitting/CDCKarimakiFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>

#include <tracking/trackFindingCDC/eventdata/hits/ConformalCDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/collections/CDCTrackList.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


void TrackProcessor::addCandidateFromHitsWithPostprocessing(std::vector<const CDCWireHit*>& hits,
                                                            const std::vector<ConformalCDCWireHit>& conformalCDCWireHitList,
                                                            CDCTrackList& cdcTrackList)
{
  if (hits.size() == 0) return;

  CDCObservations2D observations;
  for (const CDCWireHit* item : hits) {
    observations.append(*item);
  }

  const CDCRiemannFitter& fitter = CDCRiemannFitter::getFitter();
  const CDCTrajectory2D& trackTrajectory2D = fitter.fit(observations);
  CDCTrajectory3D trajectory3D(trackTrajectory2D, CDCTrajectorySZ::basicAssumption());

  CDCTrack& track = cdcTrackList.createEmptyTrack();
  track.setStartTrajectory3D(trajectory3D);

  TrackCreator trackCreator;
  trackCreator.create(hits, track);

  postprocessTrack(track, conformalCDCWireHitList, cdcTrackList);
}

void TrackProcessor::postprocessTrack(CDCTrack& track, const std::vector<ConformalCDCWireHit>& conformalCDCWireHitList,
                                      CDCTrackList& cdcTrackList)
{
  TrackQualityTools::normalizeTrack(track);

  HitProcessor::splitBack2BackTrack(track);
  TrackQualityTools::normalizeTrack(track);
  if (not checkTrackQuality(track, cdcTrackList)) {
    return;
  }

  HitProcessor::deleteHitsFarAwayFromTrajectory(track);
  TrackQualityTools::normalizeTrack(track);
  if (not checkTrackQuality(track, cdcTrackList)) {
    return;
  }

  HitProcessor::assignNewHitsToTrack(track, conformalCDCWireHitList);
  TrackQualityTools::normalizeTrack(track);

  HitProcessor::splitBack2BackTrack(track);
  TrackQualityTools::normalizeTrack(track);
  if (not checkTrackQuality(track, cdcTrackList)) {
    return;
  }

  HitProcessor::deleteHitsFarAwayFromTrajectory(track);
  TrackQualityTools::normalizeTrack(track);
  if (not checkTrackQuality(track, cdcTrackList)) {
    return;
  }

  HitProcessor::assignNewHitsToTrack(track, conformalCDCWireHitList);
  TrackQualityTools::normalizeTrack(track);

  HitProcessor::unmaskHitsInTrack(track);
}

bool TrackProcessor::checkTrackQuality(const CDCTrack& track, CDCTrackList& cdcTrackList)
{
  if (track.size() < 5) {
    for (const CDCRecoHit3D& hit : track) {
      hit.getWireHit().getAutomatonCell().setMaskedFlag(true);
      hit.getWireHit().getAutomatonCell().setTakenFlag(false);
    }
    cdcTrackList.getCDCTracks().remove(track);
    return false;
  }

  return true;
}

void TrackProcessor::assignNewHits(const std::vector<ConformalCDCWireHit>& conformalCDCWireHitList, CDCTrackList& cdcTrackList)
{
  cdcTrackList.getCDCTracks().erase(std::remove_if(cdcTrackList.getCDCTracks().begin(), cdcTrackList.getCDCTracks().end(),
  [](const CDCTrack & track) {
    return track.size() == 0;
  }) , cdcTrackList.getCDCTracks().end());

  cdcTrackList.doForAllTracks([&](CDCTrack & track) {

    if (track.size() < 4) return;

    HitProcessor::assignNewHitsToTrack(track, conformalCDCWireHitList);
    TrackQualityTools::normalizeTrack(track);

    std::vector<const CDCWireHit*> splittedHits = HitProcessor::splitBack2BackTrack(track);
    TrackQualityTools::normalizeTrack(track);

    addCandidateFromHitsWithPostprocessing(splittedHits, conformalCDCWireHitList, cdcTrackList);

    HitProcessor::deleteHitsFarAwayFromTrajectory(track);
    TrackQualityTools::normalizeTrack(track);
  });

  HitProcessor::reassignHitsFromOtherTracks(cdcTrackList);

  cdcTrackList.doForAllTracks([](CDCTrack & cand) {
    cand.forwardTakenFlag();
  });
}

void TrackProcessor::deleteTracksWithLowFitProbability(CDCTrackList& cdcTrackList, double minimal_probability_for_good_fit)
{
  const CDCKarimakiFitter& trackFitter = CDCKarimakiFitter::getFitter();

  cdcTrackList.getCDCTracks().erase(std::remove_if(cdcTrackList.getCDCTracks().begin(),
  cdcTrackList.getCDCTracks().end(), [&](CDCTrack & track) {
    std::vector<const CDCWireHit*> wireHits;
    for (const CDCRecoHit3D& recoHit : track) {
      wireHits.push_back(&(recoHit.getWireHit()));
    }

    const CDCTrajectory2D& fittedTrajectory = trackFitter.fitWhithoutDriftLengthVariance(wireHits);
    const double chi2 = fittedTrajectory.getChi2();
    const int dof = track.size() - 4;

    return TMath::Prob(chi2, dof) < minimal_probability_for_good_fit;
  }), cdcTrackList.getCDCTracks().end());
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
