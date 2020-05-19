/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/gearbox/Const.h>

#include <framework/core/Module.h>
#include <string>

namespace genfit {
  class MeasuredStateOnPlane;
}

namespace Belle2 {
  class RecoTrack;

  /** Base Module estimating the track time of RecoTracks - before or after the fit.
   * For the actual time estimation, see the child modules.  */
  class BaseTrackTimeEstimatorModule : public Module {

  public:
    /** Initialize the module parameters. */
    BaseTrackTimeEstimatorModule();

    /** Initialize the needed StoreArrays and ensure they are created properly. */
    void initialize() override;

    /** Loop over all RecoTracks and set their time seed correctly. Do not forget to refit them tracks afterwards. */
    void event() override;

  private:
    /** StoreArray name of the input and output reco tracks. */
    std::string m_param_recoTracksStoreArrayName = "RecoTracks";

    /**
     * Whether to use the information in the measurements (after fit)
     * or the tracking seeds for doing the extrapolation.
     */
    bool m_param_useFittedInformation = false;

    /**
     * If you want to subtract or add a certain time, you can use this variable.
     */
    double m_param_timeOffset = 0;

    /**
     * Which PDG code to use for creating the time estimate. How this information is used
     * depends on the implementation details of the child modules.
     * Please only use the positive pdg code.
     */
    unsigned int m_param_pdgCodeToUseForEstimation = 211;

    /**
     * In cases where the readout of the trigger is not located at the trigger directly and the signal has to
     * propagate a non-vanashing distance, you can set the readout position here. Please note that you have to
     * enable this feature by using the useReadoutPosition flag. You can control the propagation speed with the
     * flag readoutPositionPropagationSpeed.
     */
    std::vector<double> m_param_readoutPosition = {0, 0, 0};

    /**
     * Enable the usage of the readout position.
     * When this feature is enabled, the length from the incident of the particle in the trigger to the position
     * set by the readoutPosition flag is calculated and using the readoutPositionPropagationSpeed, a time is
     * calculated which is used in the time estimation as an offset.
     * */
    bool m_param_useReadoutPosition = false;

    /**
     * Speed of the propagation from the hit on the trigger to the readoutPosition. Only is used when the
     * flag useReadoutPosition is enabled.
     * */
    double m_param_readoutPositionPropagationSpeed = Const::speedOfLight;

    /**
     * Private helper function which calls the estimateFlightLengthUsingFittedInformation with the correct
     * measured state on plane and computes the flight time correctly.
     */
    double estimateTimeSeedUsingFittedInformation(RecoTrack& recoTrack, const Const::ChargedStable& particleHypothesis) const;

    /**
     * Private helper function which calls the estimateFlightLengthUsingSeedInformation
     * and computes the flight time correctly.
     */
    double estimateTimeSeedUsingSeedInformation(const RecoTrack& recoTrack, const Const::ChargedStable& particleHypothesis) const;

    /// Overload this function to implement a specific extrapolation mechanism for fitted tracks. The measured state is afterwards used for a momentum calculation.
    virtual double estimateFlightLengthUsingFittedInformation(genfit::MeasuredStateOnPlane& measuredStateOnPlane) const = 0;
    /// Overload this function to implement a specific extrapolation mechanism for track seeds.
    virtual double estimateFlightLengthUsingSeedInformation(const RecoTrack& recoTrack) const = 0;
  };
}

