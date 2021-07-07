/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/general/findlets/StateRejecter.dcl.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/logging/Logger.h>


namespace Belle2 {
  template <class AState, class AFindlet>
  StateRejecter<AState, AFindlet>::StateRejecter() : Super()
  {
    Super::addProcessingSignalListener(&m_firstFilter);
    Super::addProcessingSignalListener(&m_advanceFilter);
    Super::addProcessingSignalListener(&m_secondFilter);
    Super::addProcessingSignalListener(&m_updateFilter);
    Super::addProcessingSignalListener(&m_thirdFilter);
  };

  /// Expose the parameters of the subfindlet
  template <class AState, class AFindlet>
  void StateRejecter<AState, AFindlet>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
  {
    m_firstFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed("first", prefix));
    m_advanceFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed("advance", prefix));
    m_secondFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed("second", prefix));
    m_updateFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed("update", prefix));
    m_thirdFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed("third", prefix));
  };

  template <class AState, class AFindlet>
  void StateRejecter<AState, AFindlet>::apply(const std::vector<TrackFindingCDC::WithWeight<const AState*>>& currentPath,
                                              std::vector<TrackFindingCDC::WithWeight<AState*>>& childStates)
  {
    B2DEBUG(29, "Starting with " << childStates.size() << " states.");
    m_firstFilter.apply(currentPath, childStates);
    B2DEBUG(29, "After first filter " << childStates.size() << " states.");
    m_advanceFilter.apply(currentPath, childStates);
    B2DEBUG(29, "After advance filter " << childStates.size() << " states.");
    m_secondFilter.apply(currentPath, childStates);
    B2DEBUG(29, "After second filter " << childStates.size() << " states.");
    m_updateFilter.apply(currentPath, childStates);
    B2DEBUG(29, "After update filter " << childStates.size() << " states.");
    m_thirdFilter.apply(currentPath, childStates);
    B2DEBUG(29, "After third filter " << childStates.size() << " states.");
  };
}
