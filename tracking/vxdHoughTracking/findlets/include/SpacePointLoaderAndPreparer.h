/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/datastore/StoreArray.h>
#include <framework/core/ModuleParamList.h>
#include <framework/geometry/B2Vector3.h>
#include <framework/database/DBObjPtr.h>
#include <mdst/dbobjects/BeamSpot.h>
#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/vxdHoughTracking/entities/VXDHoughState.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <vxd/geometry/GeoCache.h>

#include <string>
#include <vector>

namespace Belle2 {
  namespace vxdHoughTracking {
    /**
    * Findlet for loading SVDSpacePoints and prepare them for usage in the FastInterceptFinder2D by creating VXDHoughStates
    * in which the BeamSpotPosition is used to calculate the conformal transformed x,y coordinates and the creating pairs
    * of coordinates for finding track candidates in r-phi and r-z.
    */
    class SpacePointLoaderAndPreparer : public TrackFindingCDC::Findlet<const SpacePoint*, VXDHoughState> {
      /// Parent class
      using Super = TrackFindingCDC::Findlet<const SpacePoint*, VXDHoughState>;

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

        moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "useAllSpacePoints"), m_param_useAllSpacePoints,
                                      "Use all SVDSpacePoints for track finding or only unassigned ones?", m_param_useAllSpacePoints);
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

        DBObjPtr<BeamSpot> beamSpotDB;
        if (beamSpotDB.isValid()) {
          m_BeamSpotPosition = (*beamSpotDB).getIPPosition();
        } else {
          m_BeamSpotPosition.SetXYZ(0., 0., 0.);
        }
      }

      /// Load the SVD SpacePoints and create a VXDHoughState object for each hit
      void apply(std::vector<const SpacePoint*>& spacePoints, std::vector<VXDHoughState>& hits) override
      {
        if (m_storeSpacePoints.getEntries() == 0) return;

        hits.reserve(m_storeSpacePoints.getEntries());
        spacePoints.reserve(m_storeSpacePoints.getEntries());
        for (auto& spacePoint : m_storeSpacePoints) {
          if (not m_param_useAllSpacePoints and spacePoint.getAssignmentState() == true) {
            continue;
          }
          if (spacePoint.TimeU() >= m_param_minimumUClusterTime and
              spacePoint.TimeV() >= m_param_minimumVClusterTime and
              spacePoint.TimeU() <= m_param_maximumUClusterTime and
              spacePoint.TimeV() <= m_param_maximumVClusterTime) {
            hits.emplace_back(VXDHoughState(&spacePoint, m_BeamSpotPosition));
            spacePoints.emplace_back(&spacePoint);
          }
        }
      };

    private:
      /// StoreArray name of the input Track Store Array
      std::string m_param_SVDSpacePointStoreArrayName = "SVDSpacePoints";

      /// Minimum u cluster time
      double m_param_minimumUClusterTime = -50;
      /// Minimum v cluster time
      double m_param_minimumVClusterTime = -50;
      /// Maximum u cluster time
      double m_param_maximumUClusterTime = 30;
      /// Maximum v cluster time
      double m_param_maximumVClusterTime = 30;
      /// Use all SVDSpacePoints for track finding or only unassigned ones
      bool m_param_useAllSpacePoints = false;

      /// Input SpacePoints Store Array
      StoreArray<SpacePoint> m_storeSpacePoints;

      /// B2Vector3D actually contining the BeamSpot position. This will be passed on to the VXDHoughState for the conformal transformation
      B2Vector3D m_BeamSpotPosition;
    };

  }
}
