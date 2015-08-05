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

#include <tracking/measurementCreator/creators/BaseMeasurementCreator.h>
#include <tracking/dataobjects/RecoHitInformation.h>
#include <genfit/MeasurementFactory.h>
#include <framework/gearbox/Const.h>
#include <vector>

namespace genfit {
  class AbsMeasurement;
}

namespace Belle2 {
  class RecoTrack;

  template <class HitType, Const::EDetector detector>
  class BaseMeasurementCreatorFromHit : public BaseMeasurementCreator {
  public:
    /** Creates a MeasurementCreator which handles the creation of measurements of a given kind **/
    BaseMeasurementCreatorFromHit(const genfit::MeasurementFactory<genfit::AbsMeasurement>& measurementFactory) :
      BaseMeasurementCreator(), m_measurementFactory(measurementFactory) {}

    /** Destructor **/
    virtual ~BaseMeasurementCreatorFromHit() { }

    /** Overload this method to create measurements from a given hit **/
    virtual std::vector<genfit::AbsMeasurement*> createMeasurements(HitType* hit, const RecoTrack& recoTrack,
        const RecoHitInformation& recoHitInformation) const = 0;

  private:
    /** A reference to the prefilled measurement factory */
    const genfit::MeasurementFactory<genfit::AbsMeasurement>& m_measurementFactory;

    /** We do not need this method in this overload */
    std::vector<genfit::AbsMeasurement*> createMeasurements(const RecoTrack&, const RecoHitInformation&) const override final
    {
      return {};
    }

  protected:
    /**
     * You probably need a coordinate measurement on which you can base your measurements in createMeasurements.
     * This function uses the measurementFactory to create one.
     * Please be aware that this creates two new objects on the heap: the measurement and the track point. If you do not plan to
     * use those two in a track, please delete them!
     * @param hit
     * @param recoHitInformation
     * @return a pair of the coordinate AbsMeasurement and a TrackCandHit as pointer.
     */
    std::pair<genfit::AbsMeasurement*, genfit::TrackCandHit*> createCoordinateMeasurement(HitType* hit,
        const RecoHitInformation& recoHitInformation) const
    {
      genfit::TrackCandHit* trackCandHit = new genfit::TrackCandHit(detector, hit->getArrayIndex(), -1,
          recoHitInformation.getSortingParameter());

      genfit::AbsMeasurement* coordinateMeasurement = m_measurementFactory.createOne(trackCandHit->getDetId(), trackCandHit->getHitId(),
                                                      trackCandHit);

      return std::make_pair(coordinateMeasurement, trackCandHit);
    }
  };
}
