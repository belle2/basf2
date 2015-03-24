/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFinderOutputCombiner/TrackFinderOutputCombiner.h>
#include <framework/gearbox/Const.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/legendre/TrackFitter.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <tracking/trackFindingCDC/legendre/TrackHit.h>
#include <framework/logging/Logger.h>
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/geometry/GeoCDCCreator.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void TrackFinderOutputCombiner::resetLocalTrackCandsToCorrectCDCHits(StoreArray<genfit::TrackCand>& trackCands,
    const StoreArray<CDCHit>& notAssignedCDCHits, std::string m_param_cdcHits)
{
  for (genfit::TrackCand& trackCand2 : trackCands) {

    std::vector<int> hitIDOfTrackCand2;

    for (int cdcHitID : trackCand2.getHitIDs(Const::CDC)) {
      CDCHit* cdcHitInCDCHits2 = notAssignedCDCHits[cdcHitID];
      CDCHit* cdcHitInCDCHits1 = cdcHitInCDCHits2->getRelated<CDCHit>(m_param_cdcHits);
      if (cdcHitInCDCHits1 == nullptr) {
        B2WARNING("Can not find cdcHit in other StoreArray.")
        continue;
      }
      hitIDOfTrackCand2.push_back(cdcHitInCDCHits1->getArrayIndex());
    }

    if (hitIDOfTrackCand2.empty())
      continue;

    trackCand2.reset();

    for (int hitID : hitIDOfTrackCand2) {
      trackCand2.addHit(Const::CDC, hitID);
    }
  }
}

double TrackFinderOutputCombiner::calculateChi2(std::vector<TrackHit*>& hits)
{
  const CDCRiemannFitter& fitter = CDCRiemannFitter::getFitter();
  CDCTrajectory2D trajectory2D = fitter.fit(hits);

  double chi2 = trajectory2D.getChi2();
  return TMath::Prob(chi2, trajectory2D.getNDF());
}

double TrackFinderOutputCombiner::constructTrackCandidate(genfit::TrackCand* newResultTrack, const StoreArray<CDCHit>& cdcHits)
{
  if (newResultTrack == nullptr)
    return -1;

  std::vector<TrackFindingCDC::TrackHit*> hitsAxial;

  for (int cdcHitID : newResultTrack->getHitIDs(Const::CDC)) {
    CDCHit* cdcHit = cdcHits[cdcHitID];
    if (cdcHit->getISuperLayer() % 2 == 0) {
      TrackHit* trackHit = new TrackHit(cdcHits[cdcHitID], cdcHitID);
      hitsAxial.push_back(trackHit);
    }
  }
  if (hitsAxial.size() < 1) {
    std::for_each(hitsAxial.begin(), hitsAxial.end(), [](TrackFindingCDC::TrackHit * hit) { delete hit; });
    return -1;
  }

  TrackFindingCDC::TrackFitter fitter;
  std::pair<double, double> reference_point;
  std::pair<double, double> track_parameter;
  fitter.fitTrackCandidateFast(hitsAxial, track_parameter, reference_point);

  int charge = TrackCandidate::getChargeAssumption(track_parameter.first, track_parameter.second, hitsAxial);
  TrackCandidate trackCandidate(track_parameter.first, track_parameter.second, charge, hitsAxial);
  TVector3 momentum(trackCandidate.getMomentumEstimation(true));

  // Reuse the z information
  momentum.SetZ(newResultTrack->getMomSeed().Z());
  TVector3 position(reference_point.first, reference_point.second, newResultTrack->getPosSeed().Z());
  int pdgCode = (211) * trackCandidate.getChargeSign();
  newResultTrack->setPosMomSeedAndPdgCode(position, momentum, pdgCode);

  double chi2 = trackCandidate.getChi2() * (hitsAxial.size() - 4);

  std::for_each(hitsAxial.begin(), hitsAxial.end(), [](TrackFindingCDC::TrackHit * hit) { delete hit; });
  return TMath::Prob(chi2, hitsAxial.size() - 4);
}

std::vector<TrackFinderOutputCombiner::HitSegment> TrackFinderOutputCombiner::collectHitSegmentsOfLocalTracks(
  const StoreArray<genfit::TrackCand>& localTrackCands, const StoreArray<CDCHit>& cdcHits, int m_param_minimumDistanceBetweenHits)
{
  const int notFoundID = -1;
  std::vector<HitSegment> hitSegmentList;

  for (const genfit::TrackCand& localTrackCand : localTrackCands) {
    std::vector<int> hitIDs;
    TVector2 lastWirePosition(0, 0);
    TVector2 currentWirePosition;
    for (int cdcHitID : localTrackCand.getHitIDs(Const::CDC)) {
      CDCHit* cdcHit = cdcHits[cdcHitID];
      currentWirePosition = calculateWirePosition(cdcHit);
      if (lastWirePosition.X() != 0 or lastWirePosition.Y() != 0) {
        double distance2 = (currentWirePosition - lastWirePosition).Mod2();
        if (distance2 > m_param_minimumDistanceBetweenHits) {
          B2DEBUG(90, "Breaking up a local track.")
          hitSegmentList.emplace_back(hitIDs, BestFitInformation(notFoundID, 0));
          hitIDs.clear();
        }
      }
      hitIDs.push_back(cdcHitID);
      lastWirePosition = currentWirePosition;
    }
    if (hitIDs.size() > 0)
      hitSegmentList.push_back(std::make_pair(hitIDs, std::make_pair(notFoundID, 0)));
  }
  return hitSegmentList;
}

TVector2 TrackFinderOutputCombiner::calculateWirePosition(const CDCHit* cdcHit)
{
  // Calculate the wire position (the position of the hit)
  CDC::CDCGeometryPar& cdcg = CDC::CDCGeometryPar::Instance();
  WireID wireIdOfCDCHit(cdcHit->getISuperLayer(), cdcHit->getILayer(), cdcHit->getIWire());
  TVector3 wireBegin = cdcg.wireForwardPosition(wireIdOfCDCHit);
  TVector3 wireEnd = cdcg.wireBackwardPosition(wireIdOfCDCHit);
  double fraction = -wireBegin.z() / (wireEnd.z() - wireBegin.z());
  TVector2 positionCDCHit = TVector2(
                              wireBegin.x() + fraction * (wireEnd.x() - wireBegin.x()),
                              wireBegin.y() + fraction * (wireEnd.y() - wireBegin.y()));
  return positionCDCHit;
}
