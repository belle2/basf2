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
   * Findlet to extract the global event time using the CDC drift circles information.
   *
   * The core functionality is written and documented in the TimeExtractionUtils class.
   * This findlet just calls the extractTime function in the utilities multiply times and decides, when the
   * time extraction is good enough to finish.
   *
   * As a result of this findlet, the EventT0 StoreObjPtr is set to the extracted time.
   *
   * All the extraction is done using the RecoTracks provided to the findlet
   *
   * The module works in the following way:
   * * Use 4 defined event time seeds between T0 min and T0 max. Set the event time to this seed and extract the event
   *   time using the derivatives of chi^2 calculated in the TimeExtractionUtils after arXiv:0810.2241.
   *   This shift in event time is then applied to the event t0 for CDC and the time is extracted again.
   *   One extracted time is called "converged" or "finished", if the second derivative of chi^2 to the event time is
   *   large (which means that the measurement uncertainty on the event time is small) and the chi^2 is small too.
   *   If the fit which is needed in each step fails, do not use this extracted time.
   *   If the extracted time is not between min T0 and max T0, do also not use this extracted time.
   *
   * * If one or more of the extraction steps from above lead to a converged result, use the one with the lowest chi^2.
   *   If not, start a loop where each extraction point from above is used, the extraction is done twice again and
   *   checked, if it has converged now.
   *
   * * If no converged point is found at all, the EventT0 is not set or reset the previous value contained in EventT0 for CDC
   */
  class FullGridTrackTimeExtraction final : public TrackFindingCDC::Findlet<RecoTrack*> {

  private:
    /// Type of the base class
    using Super = TrackFindingCDC::Findlet<RecoTrack*>;

  public:
    /// Create a new instance of this module.
    FullGridTrackTimeExtraction();

    /// export all the findlet parameters
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final override;

    /// apply the grid time extraction on the provided reco tracks. wasSuccesful() will return true
    /// if the apply() method was able to successfully compute an event t0
    void apply(std::vector< RecoTrack*>&) final override;

    /// requests access to the EventT0 class
    void initialize() final override;

    /// Returns true if the last run t0 extraction was successful
    bool wasSuccessful() const;

  private:
    /// Helper Structure holding one extracted time together with their chi^2.
    struct T0Try {
      /// extracted t0 of this try
      double m_extractedT0;
      /// chi2 of this extraction
      double m_chi2;

      /// Create the class with values for extracted t0 and the chi2 of this fit
      T0Try(const double& extractedT0, const double& chi2) :
        m_extractedT0(extractedT0), m_chi2(chi2) {}

      /// Make a list of extracted times sortable by their chi^2.
      friend bool operator<(const T0Try& lhs, const T0Try& rhs)
      {
        return lhs.m_chi2 < rhs.m_chi2;
      }

    };

    /// Extract the derived chi^2 from the fittable reco tracks. Return the first and second
    /// derivative of chi2
    std::pair<double, double> extractChi2DerivativesHelper(std::vector<RecoTrack*>& recoTracks,
                                                           std::map<RecoTrack*, bool>& fittableRecoTracks);

    /// Helper function to set the value as new event t0 and extract the average chi2 of the tracks
    double setTimeAndFitTracks(double value, std::vector<RecoTrack*>& recoTracks,
                               std::map<RecoTrack*, bool>& fittableRecoTracks);

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


    /// Module parameter which sets the minimum amount of CDC hits which are required for a CDC track to be
    /// used for time extraction
    unsigned int m_param_minimalNumberCDCHits = 5;
    /// Module Parameter: Minimal shift which is allowed.
    double m_param_minimalT0Shift = -70;
    /// Module Parameter: Maximal shift which is allowed.
    double m_param_maximalT0Shift = 70;
    /// Module Parameter: Number of shifts to try out.
    unsigned int m_param_numberOfGrids = 8;
    /// Module Parameter: Whether to replace an existing time estimation or not.
    bool m_param_overwriteExistingEstimation = true;
    /// Module Parameter: Use this as sigma t0.
    double m_param_t0Uncertainty = 5.1;

    /// will get rid of most curlers from background
    double m_param_minimumTrackPt = 0.35f;

    /// Returns true if the last run t0 extraction was successful
    unsigned int m_param_maximumTracksUsed = 3;

    /// Pointer to the storage of the eventwise T0 estimatio n in the data store.
    StoreObjPtr<EventT0> m_eventT0;

    /// stores if the lsat exexcution of apply() was successful
    bool m_wasSuccesful = false;
  };
}
