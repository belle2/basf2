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
  class BaseMeasurementCreatorFromCoordinateMeasurement : public BaseMeasurementCreatorFromHit<HitType, detector> {
  public:
    BaseMeasurementCreatorFromCoordinateMeasurement(const genfit::MeasurementFactory<genfit::AbsMeasurement>& measurementFactory) :
      BaseMeasurementCreatorFromHit<HitType, detector>(measurementFactory) {}

    /** Create measurements based on coordinate measurements */
    std::vector<genfit::AbsMeasurement*> createMeasurements(HitType* hit, const RecoTrack& recoTrack,
                                                            const RecoHitInformation& recoHitInformation) const override
    {
      const std::pair<genfit::AbsMeasurement*, genfit::TrackCandHit*>& coordinateMeasurement = this->createCoordinateMeasurement(hit,
          recoHitInformation);

      return std::move(createMeasurementFromCoordinateMeasurement(hit, recoTrack, recoHitInformation, coordinateMeasurement));
    }

    /** Destructor */
    virtual ~BaseMeasurementCreatorFromCoordinateMeasurement() { }

  protected:
    /** Overload this method if you want to create measurements from coordinate measurements.
     * You have to delete the content of the coordinateMeasurement pair if you do not plan to
     * use them. This method gets called in the createMeasurements method.
     * @param hit
     * @param recoTrack
     * @param recoHitInformation
     * @param coordinateMeasurement
     * @return
     */
    virtual std::vector<genfit::AbsMeasurement*> createMeasurementFromCoordinateMeasurement(HitType* hit,
        const RecoTrack& recoTrack, const RecoHitInformation& recoHitInformation,
        const std::pair<genfit::AbsMeasurement*, genfit::TrackCandHit*>& coordinateMeasurement) const = 0;

  };
}
