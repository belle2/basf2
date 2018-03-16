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
#include <tracking/trackFindingCDC/findlets/base/StoreArrayLoader.h>

#include <string>
#include <vector>

namespace Belle2 {
  class SpacePoint;

  class ModuleParamList;

  /// Load the space points from the store array to the given vector.
  class SpacePointLoader : public TrackFindingCDC::Findlet<const SpacePoint*> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<const SpacePoint*>;

  public:
    /// Add the sub findlet as a listener
    SpacePointLoader();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Do the space point retrieval.
    void apply(std::vector<const SpacePoint*>& spacePoints) final;

  private:
    /// Findlet for actually loading the hits
    TrackFindingCDC::StoreArrayLoader<const SpacePoint> m_storeArrayLoader;

    /// Use only already used hits
    bool m_param_useAssignedHits = true;
  };
}
