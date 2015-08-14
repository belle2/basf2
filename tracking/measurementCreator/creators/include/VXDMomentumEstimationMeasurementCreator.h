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
#include <tracking/measurementCreator/measurements/PlanarVXDMomentumMeasurement.h>
#include <mdst/dataobjects/MCParticle.h>
#include <genfit/PlanarMeasurement.h>

namespace Belle2 {
  /**
   * Creator for VXDMeasurements with momentum estimation based on the dEdX information.
   */
  template <class HitType, Const::EDetector detector>
  class VXDMomentumEstimationMeasurementCreator : public BaseMeasurementCreatorFromCoordinateMeasurement<HitType, detector> {
  public:
    explicit VXDMomentumEstimationMeasurementCreator(const genfit::MeasurementFactory<genfit::AbsMeasurement>& measurementFactory) :
      BaseMeasurementCreatorFromCoordinateMeasurement<HitType, detector>(measurementFactory) {}

    virtual ~VXDMomentumEstimationMeasurementCreator() { }

    /** Set the parameters of the fit functions and whether to use the thickness or not or the tracking seeds or not */
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
      } else if (parameterName == "use_trackfinder_seeds") {
        m_useTrackFinderSeeds = std::stoi(parameterValue);
      } else if (parameterName == "use_thickness") {
        m_useThickness = std::stoi(parameterValue);
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
        genfit::AbsMeasurement* coordinateMeasurement) const override
    {
      genfit::PlanarMeasurement* planarMeasurement = dynamic_cast<genfit::PlanarMeasurement*>(coordinateMeasurement);
      if (planarMeasurement == nullptr) {
        B2FATAL("Can only add VXD hits which are based on PlanarMeasurements with momentum estimation!")
      }

      const TVector3& momentum = recoTrack.getMomentum();

      if (momentum.Mag() > m_minimumMomentum) {
        return {};
      }

      PlanarVXDMomentumMeasurement<HitType>* momentumMeasurement = new PlanarVXDMomentumMeasurement<HitType>(*planarMeasurement, hit,
          m_fitParameters, m_correctionFitParameters, m_useTrackFinderSeeds, m_useThickness);
      return {momentumMeasurement};
    }

  private:
    /** Parameters for the main function */
    typename VXDMomentumEstimation<HitType>::FitParameters m_fitParameters;
    /** Parameters for the correction function. Set them to zero to not use a correction function */
    typename VXDMomentumEstimation<HitType>::CorrectionFitParameters m_correctionFitParameters;
    /** Use the seeds of the track finder or the seeds of the MC particles */
    bool m_useTrackFinderSeeds = true;
    /** Use the thickness of the clusters of the path length for estimating dX */
    bool m_useThickness = false;
    /** Minimal value for the momentum below the estimation is used */
    double m_minimumMomentum = 0.1;
  };
}
