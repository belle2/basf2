/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/measurementCreator/creators/BaseMeasurementCreatorFromHit.h>

namespace Belle2 {
  /** A measurement creator for normal coordinate measurements */
  template <class HitType, Const::EDetector detector>
  class CoordinateMeasurementCreator : public BaseMeasurementCreatorFromHit<HitType, detector> {
  public:
    explicit CoordinateMeasurementCreator(const genfit::MeasurementFactory<genfit::AbsMeasurement>& measurementFactory) :
      BaseMeasurementCreatorFromHit<HitType, detector>(measurementFactory) {}

    /** Create measurements based on coordinate measurements */
    std::vector<genfit::TrackPoint*> createMeasurementPoints(HitType* hit, RecoTrack& recoTrack,
                                                             const RecoHitInformation& recoHitInformation) const override
    {
      genfit::AbsMeasurement* coordinateMeasurement = this->createCoordinateMeasurement(hit, recoHitInformation);

      genfit::TrackPoint* trackPoint =
        this->createTrackPointWithRecoHitInformation(coordinateMeasurement, recoTrack, recoHitInformation);

      return {trackPoint};
    }

    /** Destructor */
    virtual ~CoordinateMeasurementCreator() { }
  };
}
