/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/eventTimeExtraction/findlets/DriftLengthBasedEventTimeExtractor.h>
#include <tracking/eventTimeExtraction/findlets/BaseEventTimeExtractor.icc.h>

#include <tracking/trackFindingCDC/eventdata/utils/DriftTimeUtil.h>

#include <genfit/KalmanFitterInfo.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>
#include <tracking/dataobjects/RecoHitInformation.h>
#include <tracking/dataobjects/RecoTrack.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  void addEventT0AndWeight(const genfit::MeasuredStateOnPlane& mSoP,
                           const CDCHit& cdcHit,
                           const RecoTrack& recoTrack,
                           const RecoHitInformation* recoHitInformation,
                           std::vector<std::pair<double, double>>& eventT0WithWeights)
  {
    WireID wireID(cdcHit.getID());
    unsigned short iCLayer = wireID.getICLayer();
    const TVector3& pos = mSoP.getPos();
    const TVector3& mom = mSoP.getMom();

    // measured time = channel t0 - tdc * tdc bin width (ok)
    const bool smear = false;
    const double measuredTime = DriftTimeUtil::getMeasuredTime(wireID, cdcHit.getTDCCount(), smear);

    /*
     * prop time = distance * prop speed
     * distance = z distance * stereo factor
     * stereo factor = (forward - backward).Mag() / (forward - backward).Z()
     * z distance = z hit - backward.Z()
     * distance += backward delta Z (if ZposMode == 1)
     */
    const double propTime = DriftTimeUtil::getPropTime(wireID, pos.Z());

    double flightTime = mSoP.getTime();

    // time walk taken from geometry parameters
    const double timeWalk = DriftTimeUtil::getTimeWalk(wireID, cdcHit.getADCCount());

    // The state at index 3 is equivalent to the drift length -> see CDCRecoHit HMatrix
    // The rest is handled by the CDCGeometryPar
    const TVector3& wirePositon = mSoP.getPlane()->getO();
    const double alpha = -wirePositon.DeltaPhi(mom);
    const double theta = mom.Theta();

    const TVectorD& stateOnPlane = mSoP.getState();
    const double driftLengthEstimate = std::abs(stateOnPlane(3));
    const bool rl = stateOnPlane(3) > 0;
    const double driftTime = DriftTimeUtil::getDriftTime(driftLengthEstimate, iCLayer, rl, alpha, theta);

    // daf weight taken from track point
    const genfit::TrackPoint* trackPoint = recoTrack.getCreatedTrackPoint(recoHitInformation);
    const auto* kalmanFitterInfo = trackPoint->getKalmanFitterInfo();
    const double dafWeight = kalmanFitterInfo->getWeights().at(0);

    const double eventT0 = measuredTime - propTime - flightTime - timeWalk - driftTime;

    eventT0WithWeights.emplace_back(eventT0, dafWeight);
  }
}

void DriftLengthBasedEventTimeExtractor::apply(std::vector<RecoTrack*>& recoTracks)
{
  TrackFitter trackFitter;
  m_wasSuccessful = false;


  // TODO: in principle this should also be possible for CDCTracks
  // Collect all event t0 hypothesis with their weights
  std::vector<std::pair<double, double>> eventT0WithWeights;

  for (RecoTrack* recoTrack : recoTracks) {
    if (not trackFitter.fit(*recoTrack)) {
      continue;
    }

    const std::vector<CDCHit*>& cdcHits = recoTrack->getSortedCDCHitList();
    for (CDCHit* cdcHit : cdcHits) {
      RecoHitInformation* recoHitInformation = recoTrack->getRecoHitInformation(cdcHit);
      try {
        const genfit::MeasuredStateOnPlane& mSoP = recoTrack->getMeasuredStateOnPlaneFromRecoHit(recoHitInformation);
        addEventT0AndWeight(mSoP, *cdcHit, *recoTrack, recoHitInformation, eventT0WithWeights);
      } catch (...) {
        continue;
      }
    }
  }

  // calculate the weighted median
  std::sort(eventT0WithWeights.begin(), eventT0WithWeights.end(), [](const auto & lhs, const auto & rhs) {
    return lhs.first < rhs.first;
  });

  double extractedEventT0 = NAN;

  double weightSum = 0;
  for (const auto& pair : eventT0WithWeights) {
    weightSum += pair.second;
  }

  double cumSum = 0;
  for (const auto& pair : eventT0WithWeights) {
    cumSum += pair.second;
    if (cumSum > 0.5 * weightSum) {
      extractedEventT0 = pair.first;
      break;
    }
  }

  if (not std::isnan(extractedEventT0)) {
    EventT0::EventT0Component eventT0Component(extractedEventT0, NAN, Const::CDC, "drift length");
    m_eventT0->setEventT0(eventT0Component);
    m_wasSuccessful = true;
    B2DEBUG(50, "Drift length gave a result of " << extractedEventT0);
  } else {
    B2DEBUG(50, "Extracted event t0 is nan.");
  }
}
