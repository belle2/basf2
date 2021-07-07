/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/general/findlets/OnStateApplier.dcl.h>
#include <tracking/trackFindingCDC/numerics/WithWeight.h>
#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <vector>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  /**
   * Specialisation of the OnStateApplier, which
   * (a) uses a filter for the () operator, which is configurable
   * (b) does only allow for the best N candidates in the child states. All other states will be deleted.
   *
   * Id useNStates is 0, all states are used.
   */
  template <class AState, class AFilter>
  class LimitedOnStateApplier : public OnStateApplier<AState> {
  private:
    /// Parent class
    using Super = OnStateApplier<AState>;

    /// The object to filer
    using Object = typename Super::Object;

  public:
    /// Constructor adding the findlet as a listener.
    LimitedOnStateApplier();

    /// Apply the filter to each pair of states and current path and let only pass the best N states.
    void apply(const std::vector<TrackFindingCDC::WithWeight<const AState*>>& currentPath,
               std::vector<TrackFindingCDC::WithWeight<AState*>>& childStates) override;

    /// Copy the filter operator to this method
    TrackFindingCDC::Weight operator()(const Object& object) override;

    /// Expose the parameters of the subfindlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override ;

  private:
    /// Parameter how many objects should pass maximal
    int m_param_useNStates = 0;

    /// Filter to decide on the states
    AFilter m_filter;
  };
}
