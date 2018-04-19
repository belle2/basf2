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

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <framework/dataobjects/EventT0.h>
#include <framework/core/Module.h>

namespace Belle2 {
  class RecoTrack;

  /**
   * Comparable module to the FullGridTimeExtractionModule, but less well performing.
   *
   * Does also extract the event time from an array of reco tracks, but uses a simple iterative approach:
   * * Try to extract the event time from the reco tracks (by building an average over the whole array) and apply
   *   this shift to the tracks.
   * * Refit and extract again until the maximum number of tries is reached or the
   *   extraction has "converged", meaning the change is below 2 ns (the needed precision).
   * * If the extraction fails, because e.g. the fit fails, randomize the next time step to try out different
   *   starting parameters.
   */
  class TrackTimeExtraction final : public TrackFindingCDC::Findlet<RecoTrack*> {

  private:
    /// Type of the base class
    using Super = TrackFindingCDC::Findlet<RecoTrack*>;

  public:
    /// Create a new instance of the module.
    TrackTimeExtraction();

    /// Expose the parameters to a module
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override final;

    /// Short description of the findlet
    std::string getDescription() override final;

    /// Register the store arrays and store obj pointers.
    void initialize() override final;

    /// timing extraction for this findlet
    void apply(std::vector<RecoTrack*>&) override final;

    /// Returns true if the last run t0 extraction was successful
    bool wasSucessful() const;

  private:

    /// Module parameter: Maximal number of iterations to perform.
    unsigned int m_param_maximalIterations = 10;
    /// Module parameter: Minimal number of iterations to perform.
    unsigned int m_param_minimalIterations = 1;
    /// Module parameter: Minimal deviation between two extractions, to call the extraction as converged.
    double m_param_minimalTimeDeviation = 0.2;
    /// Module parameter: Whether to randomize the extracted time, when the fit fails.
    bool m_param_randomizeOnError = true;
    /// Module parameter: The maximal and minimal limit [-l, l] in which to randomize the extracted time on errors.
    double m_param_randomizeLimits = 70;
    /// Module parameter: Whether to replace an existing time estimation or not.
    bool m_param_overwriteExistingEstimation = true;
    /// Module parameter: Hard cut on this value of extracted times in the positive as well as the negative direction.
    double m_param_maximalExtractedT0 = 70;
    /// Module Parameter: Use this as sigma t0.
    double m_param_t0Uncertainty = 5.1;

    /// will get rid of most curlers from background
    double m_param_minimumTrackPt = 0.35;

    /// Module parameter which sets the minimum amount of CDC hits which are required for a CDC track to be
    /// used for time extraction
    unsigned int m_param_minimalNumberCDCHits = 5;

    /// the maximum amount of tracks used for the extraction
    /// Using fewer tracks speeds up the module
    unsigned int m_param_maximumTracksUsed = 3;

    /// stores if the last run of the t0 extraction was successful
    bool m_lastRunSucessful = false;

    /// Pointer to the storage of the eventwise T0 estimation in the data store.
    StoreObjPtr<EventT0> m_eventT0;

    /// Helper function doing one step of the time extraction.
    double extractTrackTime(std::vector<RecoTrack*>& recoTracks) const;
    /// Helper function doing all iteration steps of the time extraction, this function will
    /// iteratively update the EventT0 object
    void extractTrackTimeLoop(std::vector<RecoTrack*>& recoTracks);
  };
}
