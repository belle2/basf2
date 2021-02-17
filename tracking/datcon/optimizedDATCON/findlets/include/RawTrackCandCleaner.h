/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <framework/datastore/StoreArray.h>

#include <string>
#include <vector>

namespace Belle2 {
  class SpacePoint;
  class SpacePointTrackCand;
  class VxdID;

  class ModuleParamList;

  /**
   * Findlet for rejecting wrong SpacePointTrackCands and for removing bad hits.
   */
  class TrackCandidateRejecter : public
    TrackFindingCDC::Findlet<SpacePointTrackCand> {
    /// Parent class
    using Super =
      TrackFindingCDC::Findlet<SpacePointTrackCand>;

  public:
    /// Find intercepts in the 2D Hough space
    TrackCandidateRejecter();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Create the store arrays
    void initialize() override;

    /// Reject bad SpacePointTrackCands and bad hits inside the remaining
    void apply(std::vector<SpacePointTrackCand>& trackCandidates) override;

  private:

    /// the current track candidate
//     std::vector<const SpacePoint*> m_currentTrackCandidate;

    /// vector containing track candidates, consisting of the found intersection values in the Hough Space
    std::vector<SpacePointTrackCand> m_prunedTrackCandidates;

  };
}
