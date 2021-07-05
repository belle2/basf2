/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/spacePointCreation/SpacePoint.h>
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
