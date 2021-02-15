/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <framework/datastore/StoreArray.h>

#include <tracking/datcon/optimizedDATCON/entities/HitDataCache.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <framework/geometry/B2Vector3.h>

#include <string>
#include <vector>

namespace Belle2 {
  class ModuleParamList;

  /**
   * Findlet for loading SVDClusters that were created by the DATCONSVDSimpleClusterizerModule and prepare them
   * for usage in the FastInterceptFinder2D by calculating the conformal transformed x,y coordinates and the creating pairs
   * of coordinates for finding track candidates in r-phi and r-z.
   */
  class SpacePointLoaderAndPreparer : public TrackFindingCDC::Findlet<HitDataCache> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<HitDataCache>;

  public:
    /// Load clusters and prepare them for intercept finding
    SpacePointLoaderAndPreparer();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Create the store arrays
    void initialize() override;

    /// Load the SVD SpacePoints and create a HitDataCache object for each hit
    void apply(std::vector<HitDataCache>& hits) override
    {
      if (m_storeSpacePoints.getEntries() == 0) return;

      for (auto& spacePoint : m_storeSpacePoints) {
        const B2Vector3D& hitPos = spacePoint.getPosition();
        const double hitRadiusSquared = hitPos.Perp() * hitPos.Perp();
        hits.emplace_back(&spacePoint, spacePoint.getVxdID(), hitPos.X(), hitPos.Y(), hitPos.Z(),
                          2.*hitPos.X() / hitRadiusSquared, 2.*hitPos.Y() / hitRadiusSquared);
      }
    };

  private:
    // Parameters
    /// StoreArray name of the input Track Store Array
    std::string m_param_SVDSpacePointStoreArrayName = "SVDSpacePoints";

    // Store Arrays
    /// Input SpacePoints Store Array
    StoreArray<SpacePoint> m_storeSpacePoints;

  };
}
