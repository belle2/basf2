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

#include <tracking/ckf/general/findlets/SpacePointTagger.dcl.h>

#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/spacePointCreation/SpacePoint.h>

#include <framework/core/ModuleParamList.templateDetails.h>

namespace Belle2 {
  /// Clear the used clusters
  template <class AResult, class ACluster>
  void SpacePointTagger<AResult, ACluster>::beginEvent()
  {
    Super::beginEvent();

    m_usedClusters.clear();
    m_usedSpacePoints.clear();
  }

  /// Expose the parameters of the findlet
  template <class AResult, class ACluster>
  void SpacePointTagger<AResult, ACluster>::exposeParameters(ModuleParamList* moduleParamList,
                                                             const std::string& prefix)
  {
    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "singleClusterLevel"),
                                  m_param_singleClusterLevel,
                                  "Mark SP as used, if the share a single cluster with the results, or if they "
                                  "share a whole SP.",
                                  m_param_singleClusterLevel);
    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "markUsedSpacePoints"),
                                  m_param_markUsedSpacePoints,
                                  "Mark used space points as assigned.",
                                  m_param_markUsedSpacePoints);
  }

  /// Mark all space points as used, that they share clusters if the given kind with the results.
  template <class AResult, class ACluster>
  void SpacePointTagger<AResult, ACluster>::apply(const std::vector<AResult>& results,
                                                  const std::vector<const SpacePoint*>& spacePoints)
  {
    if (not m_param_markUsedSpacePoints) {
      return;
    }

    for (const AResult& result : results) {
      const std::vector<const SpacePoint*>& hits = result.getHits();
      for (const SpacePoint* spacePoint : hits) {
        m_usedSpacePoints.insert(spacePoint);

        if (not m_param_singleClusterLevel) {
          continue;
        }

        const auto& relatedClusters = spacePoint->getRelationsTo<ACluster>();
        for (const ACluster& relatedCluster : relatedClusters) {
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

      const auto& relatedClusters = spacePoint->getRelationsTo<ACluster>();
      for (const ACluster& relatedCluster : relatedClusters) {
        if (TrackFindingCDC::is_in(&relatedCluster, m_usedClusters)) {
          spacePoint->setAssignmentState(true);
          break;
        }
      }
    }
  }
}
