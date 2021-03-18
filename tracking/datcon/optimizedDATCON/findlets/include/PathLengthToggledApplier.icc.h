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

#include <tracking/datcon/optimizedDATCON/findlets/PathLengthToggledApplier.dcl.h>
#include <tracking/datcon/optimizedDATCON/findlets/OnHitApplier.icc.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

namespace Belle2 {

  /// Add the subfilters as listeners.
  template <class AHit, class AFindlet>
  PathLengthToggledApplier<AHit, AFindlet>::PathLengthToggledApplier() : Super()
  {
    Super::addProcessingSignalListener(&m_threeHitFilterFindlet);
    Super::addProcessingSignalListener(&m_fourHitFilterFindlet);
    Super::addProcessingSignalListener(&m_pathFilterFindlet);
  }

  /// Expose parameters of the subfilters and the layer to change.
  template <class AHit, class AFindlet>
  void PathLengthToggledApplier<AHit, AFindlet>::exposeParameters(ModuleParamList* moduleParamList,
      const std::string& prefix)
  {
    m_threeHitFilterFindlet.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "threeHit"));
    m_fourHitFilterFindlet.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "fourHit"));
    m_pathFilterFindlet.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "shortPath"));
  }

  /// The weight is calculated using the subfilter based on the geometrical layer of the state.
  template <class AHit, class AFindlet>
  void PathLengthToggledApplier<AHit, AFindlet>::apply(const std::vector<TrackFindingCDC::WithWeight<const AHit*>>& currentPath,
                                                       std::vector<TrackFindingCDC::WithWeight<AHit*>>& childHits)
  {
    // if currentPath.size() == 2, including the single child hits makes the total path length 3, so use the threeHitFilter
    // if currentPath.size() == 3, including the single child hits makes the total path length 4, so use the fourHitFilter
    // if currentPath.size() > 3, including the single child hits makes the total path length > 4, so use the pathFilter
    if (currentPath.size() == 2) {
      m_threeHitFilterFindlet.apply(currentPath, childHits);
    } else if (currentPath.size() == 3) {
      m_fourHitFilterFindlet.apply(currentPath, childHits);
    } else if (currentPath.size() > 3) {
      m_pathFilterFindlet.apply(currentPath, childHits);
    }
  }
}
