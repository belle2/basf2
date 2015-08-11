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
  template <class HitType, Const::EDetector detector>
  class CoordinateMeasurementCreator : public BaseMeasurementCreatorFromHit<HitType, detector> {
  public:
    explicit CoordinateMeasurementCreator(const genfit::MeasurementFactory<genfit::AbsMeasurement>& measurementFactory) :
      BaseMeasurementCreatorFromHit<HitType, detector>(measurementFactory) {}

    /** Create measurements based on coordinate measurements */
    std::vector<genfit::AbsMeasurement*> createMeasurements(HitType* hit, const RecoTrack&,
                                                            const RecoHitInformation& recoHitInformation) const override
    {
      const std::pair<genfit::AbsMeasurement*, genfit::TrackCandHit*>& coordinateMeasurement = this->createCoordinateMeasurement(hit,
          recoHitInformation);

      genfit::AbsMeasurement* absCoordinateMeasurement = coordinateMeasurement.first;

      return {absCoordinateMeasurement};
    }

    /** Destructor */
    virtual ~CoordinateMeasurementCreator() { }
  };
}
