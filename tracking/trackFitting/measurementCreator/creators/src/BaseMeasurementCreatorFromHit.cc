/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFitting/measurementCreator/creators/BaseMeasurementCreatorFromHit.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <genfit/TrackPoint.h>
#include <genfit/WireTrackCandHit.h>

using namespace Belle2;

template <class HitType, Const::EDetector detector>
BaseMeasurementCreatorFromHit<HitType, detector>::BaseMeasurementCreatorFromHit(const
    genfit::MeasurementFactory<genfit::AbsMeasurement>& measurementFactory) :
  BaseMeasurementCreator(), m_measurementFactory(measurementFactory) {}

template <class HitType, Const::EDetector detector>
genfit::AbsMeasurement* BaseMeasurementCreatorFromHit<HitType, detector>::createCoordinateMeasurement(HitType* hit,
    const RecoHitInformation& /*recoHitInformation*/) const
{
  // No one is using the trackCandHit (neither in SVDRecoHit nor in PXDRecoHit). So we do not provide any here!
  genfit::AbsMeasurement* coordinateMeasurement = m_measurementFactory.createOne(detector, hit->getArrayIndex(),
                                                  nullptr);

  return coordinateMeasurement;
}

template <class HitType, Const::EDetector detector>
genfit::TrackPoint* BaseMeasurementCreatorFromHit<HitType, detector>::createTrackPointWithRecoHitInformation(
  genfit::AbsMeasurement* coordinateMeasurement,
  RecoTrack& recoTrack, const RecoHitInformation& recoHitInformation) const
{
  genfit::TrackPoint* coordinateTrackPoint = new genfit::TrackPoint(coordinateMeasurement,
      &RecoTrackGenfitAccess::getGenfitTrack(recoTrack));
  coordinateTrackPoint->setSortingParameter(recoHitInformation.getSortingParameter());

  return coordinateTrackPoint;
}

template class Belle2::BaseMeasurementCreatorFromHit<RecoHitInformation::UsedCDCHit, Const::CDC>;
template class Belle2::BaseMeasurementCreatorFromHit<RecoHitInformation::UsedSVDHit, Const::SVD>;
template class Belle2::BaseMeasurementCreatorFromHit<RecoHitInformation::UsedPXDHit, Const::PXD>;
template class Belle2::BaseMeasurementCreatorFromHit<RecoHitInformation::UsedBKLMHit, Const::BKLM>;
template class Belle2::BaseMeasurementCreatorFromHit<RecoHitInformation::UsedEKLMHit, Const::EKLM>;

template<>
genfit::AbsMeasurement* CDCBaseMeasurementCreator::createCoordinateMeasurement(RecoHitInformation::UsedCDCHit* cdcHit,
    const RecoHitInformation& recoHitInformation) const
{

  // From the genfit code: < 0 means left, > 0 means right, = 0 is undefined
  signed char rightLeftGenfitInformation = 0;
  RecoHitInformation::RightLeftInformation rightLeftHitInformation = recoHitInformation.getRightLeftInformation();
  if (rightLeftHitInformation == RecoHitInformation::RightLeftInformation::c_left) {
    rightLeftGenfitInformation = -1;
  } else if (rightLeftHitInformation == RecoHitInformation::RightLeftInformation::c_right) {
    rightLeftGenfitInformation = 1;
  }

  // No one is using the WireTrackCandHit after this stage, as the CDCRecoHit does not store it (it is just needed or getting the rl info).
  auto trackCandHit = std::unique_ptr<genfit::WireTrackCandHit>(new genfit::WireTrackCandHit(Const::CDC, cdcHit->getArrayIndex(), -1,
                      recoHitInformation.getSortingParameter(),
                      rightLeftGenfitInformation));

  genfit::AbsMeasurement* coordinateMeasurement = m_measurementFactory.createOne(trackCandHit->getDetId(), trackCandHit->getHitId(),
                                                  trackCandHit.get());



  return coordinateMeasurement;
}
