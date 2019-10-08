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

#include <tracking/trackFitting/measurementCreator/creators/BaseMeasurementCreatorFromCoordinateMeasurement.h>
#include <tracking/vxdMomentumEstimation/VXDMomentumEstimation.h>
#include <tracking/trackFitting/measurementCreator/measurements/PlanarVXDMomentumMeasurement.h>
#include <genfit/PlanarMeasurement.h>

namespace Belle2 {
  /**
   * Creator for VXDMeasurements with momentum estimation based on the dEdX information.
   * Can use different possibilities to calculate the path length needed for the dEdX calculation.
   * Needs the transformation fit parameters fitParameters and correctionFitParameters
   * which can be calculated using the IPython notebook Analyse_Hitwise.ipynb.
   */
  template <class HitType, Const::EDetector detector>
  class VXDMomentumEstimationMeasurementCreator : public BaseMeasurementCreatorFromCoordinateMeasurement<HitType, detector> {
  public:
    /** Constructor */
    explicit VXDMomentumEstimationMeasurementCreator(const genfit::MeasurementFactory<genfit::AbsMeasurement>& measurementFactory) :
      BaseMeasurementCreatorFromCoordinateMeasurement<HitType, detector>(measurementFactory) {}

    /** Desctructor. */
    virtual ~VXDMomentumEstimationMeasurementCreator() { }

    /** Set the parameters of the fit functions and whether to use the thickness or not or the tracking seeds or not. */
    void setParameter(const std::string& parameterName, const std::string& parameterValue) override
    {
      if (parameterName == "minimumMomentum") {
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
      } else if (parameterName == "useMCInformation") {
        m_useMCInformation = std::stoi(parameterValue);
      } else if (parameterName == "useThickness") {
        m_useThickness = std::stoi(parameterValue);
      } else if (parameterName == "sigma") {
        m_sigma = std::stod(parameterValue);
      } else if (parameterName == "useTrackingSeeds") {
        m_useTrackingSeeds = std::stoi(parameterValue);
      } else {
        B2FATAL("A parameter with the name " << parameterName << " and the value " << parameterValue << " could not be set.");
      }
    }

  protected:
    /**
     * Create a measurement based on the momentum estimation given by the VXDMomentumEstimation class.
     */
    virtual std::vector<genfit::AbsMeasurement*> createMeasurementFromCoordinateMeasurement(HitType* hit,
        const RecoTrack& recoTrack, const RecoHitInformation&,
        genfit::AbsMeasurement* coordinateMeasurement) const override
    {
      genfit::PlanarMeasurement* planarMeasurement = dynamic_cast<genfit::PlanarMeasurement*>(coordinateMeasurement);
      if (planarMeasurement == nullptr) {
        B2FATAL("Can only add VXD hits which are based on PlanarMeasurements with momentum estimation!");
        // Make CPP check happy
        return {};
      }

      const TVector3& momentum = recoTrack.getMomentumSeed();

      if (momentum.Mag() > m_minimumMomentum) {
        return {};
      }

      // cppcheck-suppress nullPointerRedundantCheck
      PlanarVXDMomentumMeasurement<HitType>* momentumMeasurement = new PlanarVXDMomentumMeasurement<HitType>(*planarMeasurement, hit,
          &recoTrack);
      momentumMeasurement->setCorrectionFitParameters(m_correctionFitParameters);
      momentumMeasurement->setFitParameters(m_fitParameters);
      momentumMeasurement->setSigma(m_sigma);
      momentumMeasurement->setUseMCInformation(m_useMCInformation);
      momentumMeasurement->setUseThickness(m_useThickness);
      momentumMeasurement->setUseTrackingSeeds(m_useTrackingSeeds);
      return {momentumMeasurement};
    }

  private:
    /** Parameters for the main function. */
    typename VXDMomentumEstimation<HitType>::FitParameters m_fitParameters;
    /** Parameters for the correction function. Set them to zero to not use a correction function. */
    typename VXDMomentumEstimation<HitType>::CorrectionFitParameters m_correctionFitParameters;
    /** Use the seeds of the track finder or the seeds of the MC particles. */
    bool m_useMCInformation = false;
    /** Use the thickness of the clusters of the path length for estimating dX. */
    bool m_useThickness = false;
    /** Minimal value for the momentum below the estimation is used. */
    double m_minimumMomentum = 0.1;
    /** Sigma of the measurement. */
    double m_sigma = 0.03;
    /** Use the tracking seeds in the origin for calculating the path length rather than the current state. */
    bool m_useTrackingSeeds = false;
  };

  /// Momentum measurement creator for the SVD.
  using SVDMomentumMeasurementCreator = VXDMomentumEstimationMeasurementCreator<RecoHitInformation::UsedSVDHit, Const::SVD>;
  /// Momentum measurement creator for the PXD.
  using PXDMomentumMeasurementCreator = VXDMomentumEstimationMeasurementCreator<RecoHitInformation::UsedPXDHit, Const::PXD>;
}
