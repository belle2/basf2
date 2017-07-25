/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/spacePointCreation/SpacePoint.h>

#include <framework/core/ModuleParamList.h>

namespace Belle2 {
  template <class AResultObject, class ACluster>
  class SpacePointTagger : public TrackFindingCDC::Findlet<const AResultObject, const SpacePoint* const> {
  public:
    /// Clear the used clusters
    void beginEvent() override
    {
      m_usedClusters.clear();
      m_usedSpacePoints.clear();
    }

    /// Expose the parameters of the findlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "singleClusterLevel"), m_param_singleClusterLevel,
                                    "Mark SP as used, if the share a single cluster with the results, or if they share a whole SP.",
                                    m_param_singleClusterLevel);
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "markUsedSpacePoints"), m_param_markUsedSpacePoints,
                                    "Mark used spacepoints as assigned.",
                                    m_param_markUsedSpacePoints);
    }

    /// Mark all space points as used, that they share clusters if the given kind with the results.
    void apply(const std::vector<AResultObject>& results, const std::vector<const SpacePoint*>& spacePoints) override
    {
      if (not m_param_markUsedSpacePoints) {
        return;
      }

      for (const auto& result : results) {
        const auto& hits = result.getHits();
        for (const SpacePoint* spacePoint : hits) {
          m_usedSpacePoints.insert(spacePoint);

          if (not m_param_singleClusterLevel) {
            continue;
          }

          const auto& relatedClusters = spacePoint->getRelationsWith<SVDCluster>();
          for (const auto& relatedCluster : relatedClusters) {
            m_usedClusters.insert(&relatedCluster);
          }
        }
      }

      for (const SpacePoint* spacePoint : spacePoints) {

        if (TrackFindingCDC::is_in(spacePoint, m_usedSpacePoints)) {
          spacePoint->setAssignmentState(true);
          continue;
        }

        if (not m_param_singleClusterLevel) {
          continue;
        }

        const auto& relatedClusters = spacePoint->getRelationsWith<SVDCluster>();
        for (const auto& relatedCluster : relatedClusters) {
          if (TrackFindingCDC::is_in(&relatedCluster, m_usedClusters)) {
            spacePoint->setAssignmentState(true);
            break;
          }
        }

      }
    }

  private:
    // Object Pool
    /// Store the used clusters in the results
    std::set<const ACluster*> m_usedClusters;
    /// Store the used space points in the results
    std::set<const SpacePoint*> m_usedSpacePoints;

    // Parameters
    /// Parameter: Mark SP as used, if the share a single cluster with the results, or if they share a whole SP.
    bool m_param_singleClusterLevel = true;
    /// Parameter: Mark used spacepoints as assigned
    bool m_param_markUsedSpacePoints = true;
  };
}
