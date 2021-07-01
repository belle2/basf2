/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/vxdHoughTracking/findlets/PathLengthToggledApplier.dcl.h>
#include <tracking/vxdHoughTracking/findlets/OnHitApplier.icc.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

namespace Belle2 {
  namespace vxdHoughTracking {

    /// Add the subfilters as listeners.
    template <class AHit, class AFindlet>
    PathLengthToggledApplier<AHit, AFindlet>::PathLengthToggledApplier() : Super()
    {
      Super::addProcessingSignalListener(&m_twoHitFilterFindlet);
      Super::addProcessingSignalListener(&m_threeHitFilterFindlet);
      Super::addProcessingSignalListener(&m_fourHitFilterFindlet);
      Super::addProcessingSignalListener(&m_fiveHitFilterFindlet);
    }

    /// Expose parameters of the subfilters and the layer to change.
    template <class AHit, class AFindlet>
    void PathLengthToggledApplier<AHit, AFindlet>::exposeParameters(ModuleParamList* moduleParamList,
        const std::string& prefix)
    {
      m_twoHitFilterFindlet.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "twoHit"));
      m_threeHitFilterFindlet.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "threeHit"));
      m_fourHitFilterFindlet.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "fourHit"));
      m_fiveHitFilterFindlet.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "fiveHit"));
    }

    /// The weight is calculated using the subfilter based on the geometrical layer of the state.
    template <class AHit, class AFindlet>
    void PathLengthToggledApplier<AHit, AFindlet>::apply(const std::vector<TrackFindingCDC::WithWeight<const AHit*>>& currentPath,
                                                         std::vector<TrackFindingCDC::WithWeight<AHit*>>& childHits)
    {
      // if currentPath.size() == 1, including the single child hits makes the total path length 2, so use the twoHitFilter
      // if currentPath.size() == 2, including the single child hits makes the total path length 3, so use the threeHitFilter
      // if currentPath.size() == 3, including the single child hits makes the total path length 4, so use the fourHitFilter
      // if currentPath.size() == 4, including the single child hits makes the total path length 5, so use the fiveHitFilter
      // in the rare case that currentPath already contains five or more hits, it's very likely already that it's a valid track
      // candidate, so no filter is applied here, but only the best N candidates sorted by quality are used further.
      if (currentPath.size() == 1) {
        m_twoHitFilterFindlet.apply(currentPath, childHits);
      } else if (currentPath.size() == 2) {
        m_threeHitFilterFindlet.apply(currentPath, childHits);
      } else if (currentPath.size() == 3) {
        m_fourHitFilterFindlet.apply(currentPath, childHits);
      } else if (currentPath.size() == 4) {
        m_fiveHitFilterFindlet.apply(currentPath, childHits);
      }
    }

  }
}
