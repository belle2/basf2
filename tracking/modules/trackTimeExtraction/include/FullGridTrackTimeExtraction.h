/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2016  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tobias Schlüter, Thomas Hauth, Nils Braun                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/datastore/StoreObjPtr.h>

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <framework/dataobjects/EventT0.h>
#include <framework/core/Module.h>

namespace Belle2 {
  class RecoTrack;

  /**
   * Module to extract the global event time using the CDC drift circles information.
   *
   * The core functionality is written and documented in the TimeExtractionUtils class.
   * This module just calls the extractTime function in the utilities multiply times and decides, when the
   * time extraction is good enough to finish.
   *
   * As a result of this module, the EventT0 StoreObjPtr is set to the extracted time.
   *
   * All the extraction is done using the RecoTracks stored in the given StoreArray.
   *
   * The module works in the following way:
   * * Use 4 defined event time seeds between T0 min and T0 max. Set the event time to this seed and extract the event
   *   time using the derivatives of chi^2 calculated in the TimeExtractionUtils after arXiv:0810.2241.
   *   This shift in event time is then applied to the reco tracks and the time is extracted again.
   *   One extracted time is called "converged" or "finished", if the second derivitive of chi^2 to the event time is
   *   large (which means that the measurement uncertainty on the event time is small) and the chi^2 is small too.
   *   If the fit which is needed in each step fails, do not use this extracted time.
   *   If the extracted time is not between min T0 and max T0, do also not use this extracted time.
   *
   * * If one or more of the extraction steps from above lead to a converged result, use the one with the lowest chi^2.
   *   If not, start a loop where each extraction point from above is used, the extraction is done twice again and
   *   checked, if it has converged now.
   *
   * * If no converged point is found at all, set the EventT0 to 0.
   */
  class FullGridTrackTimeExtraction : public TrackFindingCDC::Findlet<RecoTrack*> {

  private:
    /// Type of the base class
    using Super = TrackFindingCDC::Findlet<RecoTrack*>;

  public:
    /// Create a new instance of this module.
    FullGridTrackTimeExtraction();

    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final override;

    void apply(std::vector< RecoTrack*>&) final override;

    bool wasSuccessful() const;

  private:
    /// StoreArray name from which to read the reco tracks.
    //std::string m_param_recoTracksStoreArrayName = "__SelectedRecoTracks";

    /// Helper Structure holding one extracted time together with their chi^2.
    struct T0Try {
      double m_extractedT0;
      double m_chi2;

      T0Try(const double& extractedT0, const double& chi2) :
        m_extractedT0(extractedT0), m_chi2(chi2) {}

      /// Make a list of extracted times sortable by their chi^2.
      friend bool operator<(const T0Try& lhs, const T0Try& rhs)
      {
        return lhs.m_chi2 < rhs.m_chi2;
      }

    };


    std::pair<double, double> extractChi2DerivativesHelper(std::vector<RecoTrack*>& recoTracks,
                                                           std::map<RecoTrack*, bool>& fittableRecoTracks,
                                                           const unsigned int numberOfFittableRecoTracks);
    /// Extract the chi^2 from the fittable reco tracks
    double extractChi2Helper(std::vector<RecoTrack*>& recoTracks, std::map<RecoTrack*, bool>& fittableRecoTracks,
                             const unsigned int numberOfFittableRecoTracks);

    /// Helper function to add the "value" to the reco track time seeds and fit them
    void setTimeAndFitTracks(double value, std::vector<std::pair<RecoTrack*, double>>& recoTracksWithInitialValue,
                             std::map<RecoTrack*, bool>& fittableRecoTracks,
                             unsigned int& numberOfFittableRecoTracks);

    /**
     * Main function used in the module: Repeat the time extraction of the reco tracks starting from the given start
     * value until:
     * (a) the maximum number of steps is reached
     * (b) the extracted time is not in the range [t0 min, t0 max]
     * (c) the result is non-sense (e.g. NaN)
     *
     * Each extracted time is either added to the tries or the converged tries list, depending on if the chi^2 is small
     * and d^2 chi^2 / (d alpha)^2 is large enough.
     *
     * In the end, the tracks are reset to have the initial time seed.
     */
    void extractTrackTimeFrom(std::vector<RecoTrack*>& recoTracks, const double& startValue, const unsigned int steps,
                              std::vector<T0Try>& tries, std::vector<T0Try>& convergedTries,
                              const double& minimalT0, const double& maximalT0);


    unsigned int m_param_minimalNumberCDCHits = 20;
    /// Module Parameter: Minimal shift which is allowed.
    double m_param_minimalT0Shift = -70;
    /// Module Parameter: Maximal shift which is allowed.
    double m_param_maximalT0Shift = 70;
    /// Module Parameter: Number of shifts to try out.
    double m_param_numberOfGrids = 8;
    /// Module Parameter: Whether to replace an existing time estimation or not.
    bool m_param_overwriteExistingEstimation = true;
    /// Module Parameter: Use this as sigma t0.
    double m_param_t0Uncertainty = 5.1;

    /// Pointer to the storage of the eventwise T0 estimatio n in the data store.
    StoreObjPtr<EventT0> m_eventT0;

    bool m_wasSuccesful = false;
  };
}
