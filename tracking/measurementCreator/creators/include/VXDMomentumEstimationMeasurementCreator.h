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
#include <tracking/vxdMomentumEstimation/PlanarMomentumMeasurement.h>
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
      if (parameterName == "MinimumMomentum") {
        m_minimumMomentum = std::stod(parameterValue);
      } else if (parameterName == "aE") {
        m_fitParameters.aE = std::stod(parameterValue);
      } else if (parameterName == "bE") {
        m_fitParameters.bE = std::stod(parameterValue);
      } else if (parameterName == "cE") {
        m_fitParameters.cE = std::stod(parameterValue);
      } else if (parameterName == "dE") {
        m_fitParameters.dE = std::stod(parameterValue);
      } else if (parameterName == "aM") {
        m_correctionFitParameters.aM = std::stod(parameterValue);
      } else if (parameterName == "bM") {
        m_correctionFitParameters.bM = std::stod(parameterValue);
      } else if (parameterName == "cM") {
        m_correctionFitParameters.cM = std::stod(parameterValue);
      } else if (parameterName == "dM") {
        m_correctionFitParameters.dM = std::stod(parameterValue);
      } else if (parameterName == "use_seeds") {
        m_useSeeds = std::stoi(parameterValue);
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

      if (momentum.Mag() < m_minimumMomentum) {
        TVectorD rawHitCoordinates(1);
        double estimatedMomentum = 0;

        if (m_useSeeds) {
          const TVector3& momentum = recoTrack.getMomentum();
          const TVector3& position = recoTrack.getPosition();
          short charge = recoTrack.getCharge();
          estimatedMomentum = momentumEstimation.estimateQOverP(*hit, momentum, position, charge, m_fitParameters, m_correctionFitParameters);
        } else {
          MCParticle* relatedMCParticle = hit->getRelated<MCParticle>("MCParticles");
          if (relatedMCParticle == nullptr) {
            return {};
          } else {
            const TVector3& momentum = relatedMCParticle->getMomentum();
            const TVector3& position = relatedMCParticle->getProductionVertex();
            short charge = relatedMCParticle->getCharge();
            estimatedMomentum = momentumEstimation.estimateQOverP(*hit, momentum, position, charge, m_fitParameters, m_correctionFitParameters);
          }
        }

        if (not std::isnan(estimatedMomentum)) {
          rawHitCoordinates(0) = estimatedMomentum;

          TMatrixDSym rawHitCovariance(1);
          rawHitCovariance(0, 0) = 5;

          PlanarMomentumMeasurement* momentumMeasurement = new PlanarMomentumMeasurement(*planarMeasurement);
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
    bool m_useSeeds = true;
    double m_minimumMomentum;
  };
}
