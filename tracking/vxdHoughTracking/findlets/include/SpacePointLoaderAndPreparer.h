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

        moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "SVDSpacePointStoreArrayName"), m_SVDSpacePointStoreArrayName,
                                      "Name of the SVDSpacePoints Store Array.", m_SVDSpacePointStoreArrayName);

        moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "useAllSpacePoints"), m_useAllSpacePoints,
                                      "Use all SVDSpacePoints for track finding or only unassigned ones?", m_useAllSpacePoints);
      };

      /// Create the store arrays
      void initialize() override
      {
        Super::initialize();
        m_storeSpacePoints.isRequired(m_SVDSpacePointStoreArrayName);
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
          if (not m_useAllSpacePoints and spacePoint.getAssignmentState() == true) {
            continue;
          }
          hits.emplace_back(VXDHoughState(&spacePoint, m_BeamSpotPosition));
          spacePoints.emplace_back(&spacePoint);
        }
      };

    private:
      /// StoreArray name of the input Track Store Array
      std::string m_SVDSpacePointStoreArrayName = "SVDSpacePoints";

      /// Use all SVDSpacePoints for track finding or only unassigned ones
      bool m_useAllSpacePoints = false;

      /// Input SpacePoints Store Array
      StoreArray<SpacePoint> m_storeSpacePoints;

      /// B2Vector3D actually contining the BeamSpot position. This will be passed on to the VXDHoughState for the conformal transformation
      B2Vector3D m_BeamSpotPosition;
    };

  }
}
