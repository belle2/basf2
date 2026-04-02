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
#include <tracking/trackingUtilities/numerics/Weight.h>

#include <vector>

namespace Belle2 {

  /**
   * Helper findlet which applies its () operator to all pairs of path and state with all states in the given
   * child state list. It deletes all states in the list, where the operator () return NAN.
   * Should probably be overloaded in derived classes.
   */
  template <class AState>
  class OnStateApplier : public
    TrackingUtilities::Findlet<const TrackingUtilities::WithWeight<const AState*>, TrackingUtilities::WithWeight<AState*>> {
  private:
    /// Parent class
    using Super =
      TrackingUtilities::Findlet<const TrackingUtilities::WithWeight<const AState*>, TrackingUtilities::WithWeight<AState*>>;

  public:
    /// The object this filter refers to
    using Object = std::pair<const std::vector<TrackingUtilities::WithWeight<const AState*>>, AState*>;

    /// Apply the () operator to all pairs of state and current path.
    void apply(const std::vector<TrackingUtilities::WithWeight<const AState*>>& currentPath,
               std::vector<TrackingUtilities::WithWeight<AState*>>& childStates) override;

    /// The filter operator for this class
    virtual TrackingUtilities::Weight operator()(const Object& object);
  };
}
