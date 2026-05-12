/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/findlets/base/Findlet.h>

#include <tracking/trackingUtilities/numerics/WithWeight.h>

#include <vector>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  /**
   * Reject some (or all) states from a given list of child states using 5 filters.
   * Typically the filters are applied in the order:
   *
   *    filter, extrapolation, filter, kalman update, filter
   *
   * Tor simplicity, the extrapolation and the kalman update are also implemented using filters,
   * so it is possible to use a filter factory here.
   *
   * All filters get as input the current path and the full list of child states, so it
   * is possible to look for the best N states etc. The filters themselves are responsible
   * for deleting the not-wanted states.
   */
  template <class AState, class AFilter>
  class StateRejecter : public
    TrackingUtilities::Findlet<const TrackingUtilities::WithWeight<const AState*>, TrackingUtilities::WithWeight<AState*>> {
  private:
    /// Parent class
    using Super =
      TrackingUtilities::Findlet<const TrackingUtilities::WithWeight<const AState*>, TrackingUtilities::WithWeight<AState*>>;

  public:
    /// Construct this findlet and add the subfindlet as listener
    StateRejecter();

    /// Expose the parameters of the subfindlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

    /// Apply all five filters to the child states.
    void apply(const std::vector<TrackingUtilities::WithWeight<const AState*>>& currentPath,
               std::vector<TrackingUtilities::WithWeight<AState*>>& childStates) final;

  private:
    /// State filter to decide which available continuations should be traversed next.
    AFilter m_firstFilter;
    /// State filter to advance all states.
    AFilter m_advanceFilter;
    /// State filter to decide which available continuations should be traversed next.
    AFilter m_secondFilter;
    /// State filter to kalman update all states.
    AFilter m_updateFilter;
    /// State filter to decide which available continuations should be traversed next.
    AFilter m_thirdFilter;
  };
}
