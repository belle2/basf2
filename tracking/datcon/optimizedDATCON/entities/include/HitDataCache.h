/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2021 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors:  Christian Wessel                                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <vxd/dataobjects/VxdID.h>

namespace Belle2 {
  class SpacePoint;

  /// Simple container for hit information to be used during intercept finding

  struct HitDataCache {
    /// Default constructor
    HitDataCache() = default;
    /// Move constructor needed for STL containers
    HitDataCache(HitDataCache&&) = default;
    /// No copy constructor
    HitDataCache(const HitDataCache&) = delete;
    /// No copy constructor
    HitDataCache& operator=(const HitDataCache&) = delete;
    /// Move assignment constructor
    HitDataCache& operator=(HitDataCache&&) = default;
    /// Destructor
    ~HitDataCache() = default;

    /// Constructor
    HitDataCache(const SpacePoint* spp, VxdID id, ushort layer, ushort lad, double xx, double yy, double zz,
                 double xc, double yc, double localu, double localv, double pphi, double ttheta) :
      spacePoint(spp), sensorID(id), geoLayer(layer), ladder(lad), x(xx), y(yy), z(zz),
      xConformal(xc), yConformal(yc), localNormalizedu(localu), localNormalizedv(localv), phi(pphi), theta(ttheta)
    {};

    /// Pointer to current SpacePoint to be used in all further steps of track finding
    const SpacePoint* spacePoint = nullptr;
    /// SensorID of this hit
    VxdID sensorID = VxdID(0);
    /// Geometrical Layer this state is based on
    unsigned short geoLayer = std::numeric_limits<unsigned short>::quiet_NaN();
    /// Ladder this state is based on (only use for SpacePoint based states)
    unsigned short ladder = std::numeric_limits<unsigned short>::quiet_NaN();
    /// x coordinate of this hit
    double  x = std::numeric_limits<double>::quiet_NaN();
    /// y coordinate of this hit
    double  y = std::numeric_limits<double>::quiet_NaN();
    /// z coordinate of this hit
    double  z = std::numeric_limits<double>::quiet_NaN();
    /// conformal transformed x coordinate of this hit
    double  xConformal = std::numeric_limits<double>::quiet_NaN();
    /// conformal transformed y coordinate of this hit
    double  yConformal = std::numeric_limits<double>::quiet_NaN();
    /// Local normalized uCoordinate of this state, only set if based on SpacePoint
    double localNormalizedu = std::numeric_limits<double>::quiet_NaN();
    /// Local normalized vCoordinate of this state, only set if based on SpacePoint
    double localNormalizedv = std::numeric_limits<double>::quiet_NaN();
    /// Phi value of SpacePoint
    double phi = std::numeric_limits<double>::quiet_NaN();
    /// Theta value of SpacePoint
    double theta = std::numeric_limits<double>::quiet_NaN();
  };

}
