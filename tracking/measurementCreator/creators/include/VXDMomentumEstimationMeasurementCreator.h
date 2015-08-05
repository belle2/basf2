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
      BaseMeasurementCreatorFromCoordinateMeasurement<HitType, detector>(measurementFactory), m_minimumMomentum(0.1) {}

    virtual ~VXDMomentumEstimationMeasurementCreator() { }

    void setParameter(const std::string& parameterName, const std::string& parameterValue) override
    {
      // there are only double values, so this should be ok:
      double value = std::stod(parameterValue);
      if (parameterName == "MinimumMomentum") {
        m_minimumMomentum = value;
      } else if (parameterName == "aE") {
        m_fitParameters.aE = value;
      } else if (parameterName == "bE") {
        m_fitParameters.bE = value;
      } else if (parameterName == "cE") {
        m_fitParameters.cE = value;
      } else if (parameterName == "dE") {
        m_fitParameters.dE = value;
      } else if (parameterName == "aM") {
        m_correctionFitParameters.aM = value;
      } else if (parameterName == "bM") {
        m_correctionFitParameters.bM = value;
      } else if (parameterName == "cM") {
        m_correctionFitParameters.cM = value;
      } else if (parameterName == "dM") {
        m_correctionFitParameters.dM = value;
      } else {
        B2FATAL("A parameter with the name " << parameterName << " and the value " << parameterValue << " could not be set.")
      }
    }

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

      if (momentum.Mag() < m_minimumMomentum) {
        TVectorD rawHitCoordinates(1);
        double estimatedMomentum = momentumEstimation.estimateQOverP(*hit, momentum, position, charge, m_fitParameters,
                                   m_correctionFitParameters);

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

  private:
    typename VXDMomentumEstimation<HitType>::FitParameters m_fitParameters;
    typename VXDMomentumEstimation<HitType>::CorrectionFitParameters m_correctionFitParameters;
    double m_minimumMomentum;
  };
}
