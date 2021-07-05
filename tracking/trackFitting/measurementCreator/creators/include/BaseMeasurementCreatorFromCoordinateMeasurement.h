/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFitting/measurementCreator/creators/BaseMeasurementCreatorFromHit.h>

namespace Belle2 {
  /**
  * Baseclass to create measurement track points based on the coordinate measurements.
  * Use this class as a base class as in VXDMomentumEstimationMeasurementCreator.
  * All measurements that you create by overloading the createMeasurementFromCoordinateMeasurement
  * function get their own TrackPoint.
  */
  template <class HitType, Const::EDetector detector>
  class BaseMeasurementCreatorFromCoordinateMeasurement : public BaseMeasurementCreatorFromHit<HitType, detector> {
  public:

    /** Needs the genfit MeasurementFactory for this. */
    explicit BaseMeasurementCreatorFromCoordinateMeasurement(const genfit::MeasurementFactory<genfit::AbsMeasurement>&
                                                             measurementFactory) :
      BaseMeasurementCreatorFromHit<HitType, detector>(measurementFactory) {}

    /** Create measurements based on coordinate measurements. */
    std::vector<genfit::TrackPoint*> createMeasurementPoints(HitType* hit, RecoTrack& recoTrack,
                                                             const RecoHitInformation& recoHitInformation) const override
    {
      genfit::AbsMeasurement* coordinateMeasurement = this->createCoordinateMeasurement(hit,
                                                      recoHitInformation);

      const std::vector<genfit::AbsMeasurement*>& measurements =
        this->createMeasurementFromCoordinateMeasurement(hit, recoTrack, recoHitInformation, coordinateMeasurement);

      // TODO: Do we want to create one track point for each or one track point for all?
      // TODO: This is especially important in the question to which the track point should be related!
      std::vector<genfit::TrackPoint*> trackPoints;
      trackPoints.reserve(measurements.size());
      for (genfit::AbsMeasurement* measurement : measurements) {
        trackPoints.push_back(this->createTrackPointWithRecoHitInformation(measurement, recoTrack, recoHitInformation));
      }

      return trackPoints;
    }

    /** Destructor */
    virtual ~BaseMeasurementCreatorFromCoordinateMeasurement() { }

  protected:
    /** Overload this method if you want to create measurements from coordinate measurements.
     * You have to delete the content of the coordinateMeasurement if you do not plan to
     * use them. This method gets called in the createMeasurements method.
     * @param hit
     * @param recoTrack
     * @param recoHitInformation
     * @param coordinateMeasurement
     * @return a vector of measurements that get used to create as many track points.
     */
    virtual std::vector<genfit::AbsMeasurement*> createMeasurementFromCoordinateMeasurement(HitType* hit,
        const RecoTrack& recoTrack, const RecoHitInformation& recoHitInformation,
        genfit::AbsMeasurement* coordinateMeasurement) const = 0;

  };
}
