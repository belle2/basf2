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

#include <tracking/measurementCreator/creators/BaseMeasurementCreatorFromCoordinateMeasurement.h>
#include <tracking/vxdMomentumEstimation/VXDMomentumEstimation.h>
#include <genfit/PlanarMomentumMeasurement.h>
#include <genfit/PlanarMeasurement.h>

namespace Belle2 {
  template <class HitType, Const::EDetector detector>
  class VXDMomentumEstimationMeasurementCreator : public BaseMeasurementCreatorFromCoordinateMeasurement<HitType, detector> {
  public:
    VXDMomentumEstimationMeasurementCreator(const genfit::MeasurementFactory<genfit::AbsMeasurement>& measurementFactory) :
      BaseMeasurementCreatorFromCoordinateMeasurement<HitType, detector>(measurementFactory) {}

    virtual ~VXDMomentumEstimationMeasurementCreator() { }

  protected:
    /**
     * Create a measurement based on the momentum estimation given by the VXDMomentumEstimation class
     */
    virtual std::vector<genfit::AbsMeasurement*> createMeasurementFromCoordinateMeasurement(HitType* hit,
        const RecoTrack& recoTrack, const RecoHitInformation&,
        const std::pair<genfit::AbsMeasurement*, genfit::TrackCandHit*>& coordinateMeasurement) const override
    {

      genfit::AbsMeasurement* absCoordinateMeasurement = coordinateMeasurement.first;
      genfit::PlanarMeasurement* planarMeasurement = dynamic_cast<genfit::PlanarMeasurement*>(absCoordinateMeasurement);
      if (planarMeasurement == nullptr) {
        B2FATAL("Can only add VXD hits which are based on PlanarMeasurements with momentum estimation!")
      }

      const VXDMomentumEstimation<HitType>& momentumEstimation = VXDMomentumEstimation<HitType>::getInstance();

      const TVector3& momentum = recoTrack.getMomentum();
      const TVector3& position = recoTrack.getPosition();
      short charge = recoTrack.getCharge();

      if (momentum.Mag() < 0.1) {
        TVectorD rawHitCoordinates(1);
        double estimatedMomentum = momentumEstimation.estimateQOverP(*hit, momentum, position, charge);

        if (not std::isnan(estimatedMomentum)) {
          rawHitCoordinates(0) = estimatedMomentum;

          TMatrixDSym rawHitCovariance(1);
          rawHitCovariance(0, 0) = 0.2;

          genfit::PlanarMomentumMeasurement* momentumMeasurement = new genfit::PlanarMomentumMeasurement(*planarMeasurement);
          momentumMeasurement->setRawHitCoords(rawHitCoordinates);
          momentumMeasurement->setRawHitCov(rawHitCovariance);
          return {momentumMeasurement};
        }
      }

      return {};
    }

  };
}
