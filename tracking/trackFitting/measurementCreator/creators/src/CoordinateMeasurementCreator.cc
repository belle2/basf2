/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFitting/measurementCreator/creators/CoordinateMeasurementCreator.h>

using namespace Belle2;

template <class HitType, Const::EDetector detector>
CoordinateMeasurementCreator<HitType, detector>::CoordinateMeasurementCreator(const
    genfit::MeasurementFactory<genfit::AbsMeasurement>& measurementFactory) :
  BaseMeasurementCreatorFromHit<HitType, detector>(measurementFactory) {}

template <class HitType, Const::EDetector detector>
std::vector<genfit::TrackPoint*> CoordinateMeasurementCreator<HitType, detector>::createMeasurementPoints(HitType* hit,
    RecoTrack& recoTrack,
    const RecoHitInformation& recoHitInformation) const
{
  genfit::AbsMeasurement* coordinateMeasurement = this->createCoordinateMeasurement(hit, recoHitInformation);

  genfit::TrackPoint* trackPoint =
    this->createTrackPointWithRecoHitInformation(coordinateMeasurement, recoTrack, recoHitInformation);

  return {trackPoint};
}


template class Belle2::CoordinateMeasurementCreator<RecoHitInformation::UsedCDCHit, Const::CDC>;
template class Belle2::CoordinateMeasurementCreator<RecoHitInformation::UsedSVDHit, Const::SVD>;
template class Belle2::CoordinateMeasurementCreator<RecoHitInformation::UsedPXDHit, Const::PXD>;
template class Belle2::CoordinateMeasurementCreator<RecoHitInformation::UsedBKLMHit, Const::BKLM>;
template class Belle2::CoordinateMeasurementCreator<RecoHitInformation::UsedEKLMHit, Const::EKLM>;
