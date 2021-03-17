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

#include <framework/datastore/StoreArray.h>
#include <framework/core/ModuleParamList.h>

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/datcon/optimizedDATCON/entities/HitData.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <framework/geometry/B2Vector3.h>
#include <vxd/geometry/GeoCache.h>

#include <string>
#include <vector>

namespace Belle2 {
  /**
   * Findlet for loading SVDClusters that were created by the DATCONSVDSimpleClusterizerModule and prepare them
   * for usage in the FastInterceptFinder2D by calculating the conformal transformed x,y coordinates and the creating pairs
   * of coordinates for finding track candidates in r-phi and r-z.
   */
  class SpacePointLoaderAndPreparer : public TrackFindingCDC::Findlet<HitData> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<HitData>;

  public:
    /// Load clusters and prepare them for intercept finding
    SpacePointLoaderAndPreparer() {};

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      Super::exposeParameters(moduleParamList, prefix);

      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "SVDSpacePointStoreArrayName"),
                                    m_param_SVDSpacePointStoreArrayName,
                                    "Name of the SVDSpacePoints Store Array.",
                                    m_param_SVDSpacePointStoreArrayName);
    };

    /// Create the store arrays
    void initialize() override
    {
      Super::initialize();
      m_storeSpacePoints.isRequired(m_param_SVDSpacePointStoreArrayName);
    };

    /// Load the SVD SpacePoints and create a HitData object for each hit
    void apply(std::vector<HitData>& hits) override
    {
      if (m_storeSpacePoints.getEntries() == 0) return;

      hits.reserve(m_storeSpacePoints.getEntries());
      for (auto& spacePoint : m_storeSpacePoints) {
        hits.emplace_back(HitData(&spacePoint));
      }
    };

  private:
    /// StoreArray name of the input Track Store Array
    std::string m_param_SVDSpacePointStoreArrayName = "SVDSpacePoints";

    /// Input SpacePoints Store Array
    StoreArray<SpacePoint> m_storeSpacePoints;
  };
}
