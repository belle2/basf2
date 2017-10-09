/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/numerics/WithWeight.h>

#include <vector>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  template <class AState>
  class OnStateApplier : public TrackFindingCDC::Findlet<const AState* const, TrackFindingCDC::WithWeight<AState*>> {
  private:
    /// Parent class
    using Super = TrackFindingCDC::Findlet<const AState* const, TrackFindingCDC::WithWeight<AState*>>;

  public:
    /// The object this filter refers to
    using Object = std::pair<const std::vector<const AState*>, AState*>;

    /// Construct this findlet and add the subfindlet as listener
    OnStateApplier();

    /// Expose the parameters of the subfindlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /**
     */
    void apply(const std::vector<const AState*>& currentPath,
               std::vector<TrackFindingCDC::WithWeight<AState*>>& childStates) override;

    /// Return the truth information requirement of the filter itself
    virtual bool needsTruthInformation();

    /// Copy the filter operator to this method
    virtual TrackFindingCDC::Weight operator()(const Object& object);
  };
}
