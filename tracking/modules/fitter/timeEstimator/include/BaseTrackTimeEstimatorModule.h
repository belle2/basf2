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
     * Which PDG code to use for creating the time estimate. How this information is used
     * depends on the implementation details of the child modules.
     * Please only use the positive pdg code.
     */
    unsigned int m_param_pdgCodeToUseForEstimation = 211;

    /**
     * Private helper function which calls the estimateFlightLengthUsingFittedInformation with the correct
     * measured state on plane and computes the flight time correctly.
     */
    double estimateTimeSeedUsingFittedInformation(RecoTrack& recoTrack, const Const::ChargedStable& particleHypothesis) const;

    /**
     * Private helper function which calls the estimateFlightLengthUsingSeedInformation
     * and computes the flight time correctly.
     */
    double estimateTimeSeedUsingSeedInformation(RecoTrack& recoTrack, const Const::ChargedStable& particleHypothesis) const;

    /// Overload this function to implement a specific extrapolation mechanism for fitted tracks. The measured state is afterwards used for a momentum calculation.
    virtual double estimateFlightLengthUsingFittedInformation(genfit::MeasuredStateOnPlane& measuredStateOnPlane) const = 0;
    /// Overload this function to implement a specific extrapolation mechanism for track seeds.
    virtual double estimateFlightLengthUsingSeedInformation(const RecoTrack& recoTrack) const = 0;
  };
}

