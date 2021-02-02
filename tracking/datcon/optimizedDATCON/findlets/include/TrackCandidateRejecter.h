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
  class VxdID;

  class ModuleParamList;

  /**
   * Findlet for finding intersections of sinosoidal curves in the 1D Hough space by iteratively calling
   * fastInterceptFinder1d. This is done 80 times for a subset of SVD sensors, one subset for each layer 6 sensor
   * to reduce combinatorics in the Hough Space and to improve the purity of the found track candidates.
   * The found track candidates are then clustered via a recursive search. Afterwards track candidates are formed
   * and stored in the output vector.
   */
  class TrackCandidateRejecter : public
    TrackFindingCDC::Findlet<std::vector<const SpacePoint*>> {
    /// Parent class
    using Super =
      TrackFindingCDC::Findlet<std::vector<const SpacePoint*>>;

  public:
    /// Find intercepts in the 2D Hough space
    TrackCandidateRejecter();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Create the store arrays
    void initialize() override;

    /// Load in the prepared hits and create tracks for extrapolation to PXD
    void apply(std::vector<std::vector<const SpacePoint*>>& trackCandidates) override;

  private:

    /// the current track candidate
//     std::vector<const SpacePoint*> m_currentTrackCandidate;

    /// vector containing track candidates, consisting of the found intersection values in the Hough Space
    std::vector<std::vector<const SpacePoint*>> m_prunedTrackCandidates;

  };
}
