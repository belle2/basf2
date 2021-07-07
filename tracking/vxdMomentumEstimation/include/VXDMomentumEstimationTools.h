/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/spacePointCreation/SpacePoint.h>
#include <vxd/dataobjects/VxdID.h>
#include <framework/geometry/B2Vector3.h>
#include <TVector3.h>
#include <framework/dataobjects/Helix.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <algorithm>
#include <cmath>
#include <framework/logging/Logger.h>


namespace Belle2 {
  class SVDCluster;
  class PXDCluster;

  /** Tools needed for the VXD momentum estimation to, e.g. calculate the path length or properties of the hits. Can also be used for other things. */
  template <class ClusterType>
  class VXDMomentumEstimationTools {

  private:
    /// Do not create this class.
    VXDMomentumEstimationTools() { }
    /// Do not create this class.
    VXDMomentumEstimationTools(const VXDMomentumEstimationTools&);
    /// Do not create this class.
    VXDMomentumEstimationTools& operator = (const VXDMomentumEstimationTools&);

  public:
    /** Use this class as singleton. */
    static const VXDMomentumEstimationTools& getInstance()
    {
      static VXDMomentumEstimationTools instance;
      return instance;
    }

    /** Main function: return dEdX for a cluster and the given momentum, position and charge seeds. */
    double getDEDX(const ClusterType& cluster, const TVector3& momentum, const TVector3& position, short charge) const
    {

      const Helix trajectory(position, momentum, charge, 1.5);
      const double calibratedCharge = getCalibratedCharge(cluster);
      const double pathLength = getPathLength(cluster, trajectory);

      return calibratedCharge / pathLength;
    }

    /** Return dEdX but not with dX = path length but with dX = thickness of cluster. */
    double getDEDXWithThickness(const ClusterType& cluster) const
    {
      const double calibratedCharge = getCalibratedCharge(cluster);
      const double pathLength = getThicknessOfCluster(cluster);

      return calibratedCharge / pathLength;
    }

    /** Return the thickness of a cluster. */
    double getThicknessOfCluster(const ClusterType& cluster) const
    {
      const VxdID&   vxdID = cluster.getSensorID();
      const VXD::SensorInfoBase& sensorInfoBase = VXD::GeoCache::getInstance().getSensorInfo(vxdID);
      return sensorInfoBase.getThickness();
    }

    /** Return the thickness of a cluster. */
    double getWidthOfCluster(const ClusterType& cluster) const
    {
      const VxdID&   vxdID = cluster.getSensorID();
      const VXD::SensorInfoBase& sensorInfoBase = VXD::GeoCache::getInstance().getSensorInfo(vxdID);
      return sensorInfoBase.getWidth();
    }

    /** Return the thickness of a cluster. */
    double getLengthOfCluster(const ClusterType& cluster) const
    {
      const VxdID&   vxdID = cluster.getSensorID();
      const VXD::SensorInfoBase& sensorInfoBase = VXD::GeoCache::getInstance().getSensorInfo(vxdID);
      return sensorInfoBase.getLength();
    }

    /** Returns the distance from the interaction point to the cluster in the r-phi-plane. */
    double getRadiusOfCluster(const ClusterType& cluster) const
    {
      const SpacePoint* spacePoint = cluster.template getRelated<SpacePoint>("SpacePoints");

      double radius = 0;
      if (spacePoint == nullptr) {
        // Fallback function
        VxdID vxdID = cluster.getSensorID();
        VxdID::baseType layer = vxdID.getLayerNumber();
        radius = m_layerPositions[layer - 1] / 100.0;
        B2WARNING("Could not determine SpacePoint for this cluster. Falling back to geometrical information.");
      } else {
        // The positions is the one with w = 0 which is the one on the lowest detector plane.
        radius = spacePoint->getPosition().Perp();
      }
      return radius;
    }

    /** Return the layer of the cluster. */
    VxdID::baseType getLayerOfCluster(const ClusterType& cluster) const
    {
      VxdID vxdID = cluster.getSensorID();
      VxdID::baseType layer = vxdID.getLayerNumber();
      return layer;
    }

    /** Return the ladder of the cluster. */
    VxdID::baseType getLadderOfCluster(const ClusterType& cluster) const
    {
      VxdID vxdID = cluster.getSensorID();
      VxdID::baseType ladder = vxdID.getLadderNumber();
      return ladder;
    }

    /** Return the sensor number of the cluster. */
    VxdID::baseType getSensorNumberOfCluster(const ClusterType& cluster) const
    {
      VxdID vxdID = cluster.getSensorID();
      VxdID::baseType sensorNumber = vxdID.getSensorNumber();
      return sensorNumber;
    }

    /** Return the segment number of the cluster. */
    VxdID::baseType getSegmentNumberOfCluster(const ClusterType& cluster) const
    {
      VxdID vxdID = cluster.getSensorID();
      VxdID::baseType segmentNumber = vxdID.getSegmentNumber();
      return segmentNumber;
    }

    /** Return the charge of the cluster (in ADC count) calibrated with a factor of ~0.6 for pxd hits.
     * This factor can be seen in data (and is calculated from that) and has probably something to do with
     * the different readout of the hit types. */
    double getCalibratedCharge(const ClusterType& cluster) const
    {
      const double charge = cluster.getCharge();
      const double calibration = getCalibration();

      return calibration * charge;
    }

    /** Return the momentum of the simulated MCParticle at this cluster (by using the TrueHit associated with this cluster)
     * This method is implemented for the two cluster types differently below. */
    TVector3 getEntryMomentumOfMCParticle(const ClusterType&) const
    {
      B2FATAL("Can not deal with this cluster type!");
      return TVector3();
    }

    /** Return the entry position of the simulated MCParticle at this cluster (by using the TrueHit associated with this cluster)
     * This method is implemented for the two cluster types differently below. */
    TVector3 getEntryPositionOfMCParticle(const ClusterType&) const
    {
      B2FATAL("Can not deal with this cluster type!");
      return TVector3();
    }

    /** Return the path length of a particle with the given helix that goes through the cluster.
     * If the helix does not pass the cluster, return the thickness of the cluster instead.
     * It is assumed that the cluster is passed from bottom to top and not transverse or something.
     * Also we assume every cluster to not be tilted. This is wrong for the wedge cluster.
     * We have to find a way to handle those cases correctly!
     */
    double getPathLength(const ClusterType& cluster, const Helix& trajectory) const
    {
      // This is not quite correct but we can not do better without doing an extrapolation with material effects.
      // If the distance_3D is nan, it means that the helix does not reach into the cluster or curls that much
      // that it does not reach the far end of the cluster. The first case is strange
      // because the track is associated with the cluster (so it should normally reach it). The second case should be
      // really rare (because the clusters are that thin), but we have to deal with it.
      // There is also the possibility that the track curls that much that it interacts with the same sensor twice.
      // This can not be handled properly in this stage.

      const double thickness = getThicknessOfCluster(cluster);
      const double radius = getRadiusOfCluster(cluster);
      const double width = getWidthOfCluster(cluster);
      const double length = getLengthOfCluster(cluster);

      const double perp_s_at_cluster_entry = trajectory.getArcLength2DAtCylindricalR(radius);

      if (std::isnan(perp_s_at_cluster_entry)) {
        // This case is really strange. I do not know how to deal with it probably.
        return thickness;
      }

      const TVector3& position_at_inner_radius = trajectory.getPositionAtArcLength2D(perp_s_at_cluster_entry);

      const double perp_s_at_cluster_exit = trajectory.getArcLength2DAtCylindricalR(radius + thickness);

      if (std::isnan(perp_s_at_cluster_exit)) {
        return std::min(width, length);
      }

      const TVector3& position_at_outer_radius = trajectory.getPositionAtArcLength2D(perp_s_at_cluster_exit);

      const double distance_3D = (position_at_outer_radius - position_at_inner_radius).Mag();

      return distance_3D;
    }

  private:
    /** the layer positions in the case we do not have a SpacePoint we can look at. */
    const double m_layerPositions[6] = {1.42, 2.18, 3.81, 8.0, 10.51, 13.51};

    /** For SVD the calibration is 1, for PXD (see below) it is ~0.6. */
    double getCalibration() const
    {
      return 1;
    }
  };

  /** We only need a calibration for the PXD Clusters. */
  template<>
  double VXDMomentumEstimationTools<PXDCluster>::getCalibration() const;

  /** We have to handle PXD and SVD differently here. */
  template <>
  TVector3 VXDMomentumEstimationTools<PXDCluster>::getEntryMomentumOfMCParticle(const PXDCluster& cluster) const;

  /** We have to handle PXD and SVD differently here. */
  template <>
  TVector3 VXDMomentumEstimationTools<SVDCluster>::getEntryMomentumOfMCParticle(const SVDCluster& cluster) const;

  /** We have to handle PXD and SVD differently here. */
  template <>
  TVector3 VXDMomentumEstimationTools<PXDCluster>::getEntryPositionOfMCParticle(const PXDCluster& cluster) const;

  /** We have to handle PXD and SVD differently here. */
  template <>
  TVector3 VXDMomentumEstimationTools<SVDCluster>::getEntryPositionOfMCParticle(const SVDCluster& cluster) const;
}
