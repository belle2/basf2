/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <pxd/dataobjects/PXDCluster.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>

#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <tracking/spacePointCreation/SpacePoint.h>

#include <framework/dataobjects/Helix.h>


namespace Belle2 {
  template <class ClusterType>
  class VXDMomentumEstimationTools {

  private:
    VXDMomentumEstimationTools() : m_layerPositions {1.42, 2.18, 3.81, 8.0, 10.51, 13.51} { }
    VXDMomentumEstimationTools(const VXDMomentumEstimationTools&);
    VXDMomentumEstimationTools& operator = (const VXDMomentumEstimationTools&);

  public:
    static const VXDMomentumEstimationTools& getInstance()
    {
      static VXDMomentumEstimationTools instance;
      return instance;
    }

    double getDEDX(const ClusterType& cluster, const TVector3& momentum, const TVector3& position, short charge) const
    {

      const Helix trajectory(position, momentum, charge, 1.5);
      const double calibratedCharge = getCalibratedCharge(cluster);
      const double pathLength = getPathLength(cluster, trajectory);

      return calibratedCharge / pathLength;
    }

    double getDEDXWithThickness(const ClusterType& cluster) const
    {
      const double calibratedCharge = getCalibratedCharge(cluster);
      const double pathLength = getThicknessOfCluster(cluster);

      return calibratedCharge / pathLength;
    }

    double getThicknessOfCluster(const ClusterType& cluster) const
    {
      const VxdID&   vxdID = cluster.getSensorID();
      const VXD::SensorInfoBase& sensorInfoBase = VXD::GeoCache::getInstance().getSensorInfo(vxdID);
      return sensorInfoBase.getThickness() / 100;
    }

    double getRadiusOfCluster(const ClusterType& cluster) const
    {
      const SpacePoint* spacePoint = cluster.template getRelated<SpacePoint>("SpacePoints");

      double radius = 0;
      if (spacePoint == nullptr) {
        VxdID vxdID = cluster.getSensorID();
        VxdID::baseType layer = vxdID.getLayerNumber();
        radius = m_layerPositions[layer - 1];
      } else {
        radius = spacePoint->getPosition().Perp();
      }
      return radius / 100;
    }

    double getCalibratedCharge(const ClusterType& cluster) const
    {
      const double charge = cluster.getCharge();
      const double calibration = getCalibration();

      return calibration * charge;
    }

    TVector3 getMomentumOfMCParticle(const SVDCluster& cluster) const
    {
      SVDTrueHit* trueHit = cluster.template getRelated<SVDTrueHit>("SVDTrueHits");
      const VxdID& vxdID = cluster.getSensorID();
      const VXD::SensorInfoBase& sensorInfoBase = VXD::GeoCache::getInstance().getSensorInfo(vxdID);
      const TVector3& momentum = sensorInfoBase.vectorToGlobal(trueHit->getMomentum());

      return momentum;
    }

    TVector3 getMomentumOfMCParticle(const PXDCluster& cluster) const
    {
      PXDTrueHit* trueHit = cluster.getRelated<PXDTrueHit>("PXDTrueHits");
      const VxdID& vxdID = cluster.getSensorID();
      const VXD::SensorInfoBase& sensorInfoBase = VXD::GeoCache::getInstance().getSensorInfo(vxdID);
      const TVector3& momentum = sensorInfoBase.vectorToGlobal(trueHit->getMomentum());

      return momentum;
    }

    double getPathLength(const ClusterType& cluster, const Helix& trajectory) const
    {

      const double thickness = getThicknessOfCluster(cluster);
      const double radius = getRadiusOfCluster(cluster);

      const double perp_s_at_cluster_entry = trajectory.getArcLength2DAtCylindricalR(radius - thickness / 2.0);
      const double perp_s_at_cluster_exit = trajectory.getArcLength2DAtCylindricalR(radius + thickness / 2.0);

      const TVector3& position_at_inner_radius = trajectory.getPositionAtArcLength2D(perp_s_at_cluster_entry);
      const TVector3& position_at_outer_radius = trajectory.getPositionAtArcLength2D(perp_s_at_cluster_exit);

      const double distance_3D = (position_at_outer_radius - position_at_inner_radius).Mag();

      if (std::isnan(distance_3D)) {
        // This is not quete correct but we can not do better.
        // If the distance_3D is nan, it means that the helix does not reach into the cluster. This is strange
        // because the track is associated with the cluster (so it should normally reach it).
        return thickness;
      } else {
        return distance_3D;
      }
    }

  private:
    double m_layerPositions[6];

    double getCalibration() const
    {
      return 1;
    }
  };

  template<>
  double VXDMomentumEstimationTools<PXDCluster>::getCalibration() const
  {
    return 0.653382;
  }
}
