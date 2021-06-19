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

#include <framework/datastore/StoreArray.h>
#include <framework/core/ModuleParamList.h>
#include <framework/geometry/B2Vector3.h>
#include <framework/database/DBObjPtr.h>
#include <mdst/dbobjects/BeamSpot.h>
#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/datcon/optimizedDATCON/entities/HitData.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <vxd/geometry/GeoCache.h>

#include <string>
#include <vector>

namespace Belle2 {
  /**
   * Findlet for loading SVDClusters that were created by the DATCONSVDSimpleClusterizerModule and prepare them
   * for usage in the FastInterceptFinder2D by calculating the conformal transformed x,y coordinates and the creating pairs
   * of coordinates for finding track candidates in r-phi and r-z.
   */
  class SpacePointLoaderAndPreparer : public TrackFindingCDC::Findlet<const SpacePoint*, HitData> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<const SpacePoint*, HitData>;

  public:
    /// Load clusters and prepare them for intercept finding
    SpacePointLoaderAndPreparer() {};

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      Super::exposeParameters(moduleParamList, prefix);

      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "SVDSpacePointStoreArrayName"), m_param_SVDSpacePointStoreArrayName,
                                    "Name of the SVDSpacePoints Store Array.", m_param_SVDSpacePointStoreArrayName);

      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minimumUClusterTime"), m_param_minimumUClusterTime,
                                    "Minimum time of the u cluster (in ns).", m_param_minimumUClusterTime);

      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minimumVClusterTime"), m_param_minimumVClusterTime,
                                    "Minimum time of the v cluster (in ns).", m_param_minimumVClusterTime);

      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumUClusterTime"), m_param_maximumUClusterTime,
                                    "Maximum time of the u cluster (in ns).", m_param_maximumUClusterTime);

      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumVClusterTime"), m_param_maximumVClusterTime,
                                    "Maximum time of the v cluster (in ns).", m_param_maximumVClusterTime);
    };

    /// Create the store arrays
    void initialize() override
    {
      Super::initialize();
      m_storeSpacePoints.isRequired(m_param_SVDSpacePointStoreArrayName);
    };

    /// Retrieve the BeamSpot from DB
    void beginRun() override
    {
      Super::beginRun();

      if (m_BeamSpotDB.isValid()) {
        m_BeamSpot = *m_BeamSpotDB;
        const TVector3& BeamSpotPosition = m_BeamSpot.getIPPosition();
        m_BeamSpotPosition.SetXYZ(BeamSpotPosition.X(), BeamSpotPosition.Y(), BeamSpotPosition.Z());
      } else {
        m_BeamSpotPosition.SetXYZ(0., 0., 0.);
      }
      B2INFO("DATCON uses following BeamSpot: " <<
             m_BeamSpotPosition.X() << ", " << m_BeamSpotPosition.Y() << ", " << m_BeamSpotPosition.Z());
    }

    /// Load the SVD SpacePoints and create a HitData object for each hit
    void apply(std::vector<const SpacePoint*>& spacePoints, std::vector<HitData>& hits) override
    {
      if (m_storeSpacePoints.getEntries() == 0) return;

      hits.reserve(m_storeSpacePoints.getEntries());
      spacePoints.reserve(m_storeSpacePoints.getEntries());
      for (auto& spacePoint : m_storeSpacePoints) {
        if (spacePoint.TimeU() >= m_param_minimumUClusterTime and
            spacePoint.TimeV() >= m_param_minimumVClusterTime and
            spacePoint.TimeU() <= m_param_maximumUClusterTime and
            spacePoint.TimeV() <= m_param_maximumVClusterTime) {
          hits.emplace_back(HitData(&spacePoint, m_BeamSpotPosition));
          spacePoints.emplace_back(&spacePoint);
        }
      }
    };

  private:
    /// StoreArray name of the input Track Store Array
    std::string m_param_SVDSpacePointStoreArrayName = "SVDSpacePoints";

    /// Minimum u cluster time
    double m_param_minimumUClusterTime = -25;
    /// Minimum v cluster time
    double m_param_minimumVClusterTime = -25;
    /// Maximum u cluster time
    double m_param_maximumUClusterTime = 40;
    /// Maximum v cluster time
    double m_param_maximumVClusterTime = 40;

    /// Input SpacePoints Store Array
    StoreArray<SpacePoint> m_storeSpacePoints;

    /// BeamSpot from DB
    DBObjPtr<BeamSpot> m_BeamSpotDB;
    /// Actual BeamSpot
    BeamSpot m_BeamSpot;
    /// B2Vector3D actually contining the BeamSpot position. This will be passed on to the HitData for the conformal transformation
    B2Vector3D m_BeamSpotPosition;
  };
}
