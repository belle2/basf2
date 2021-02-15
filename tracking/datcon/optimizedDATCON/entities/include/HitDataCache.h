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
    /// Constructor
    HitDataCache(const SpacePoint* spp, VxdID id, double xx, double yy, double zz, double xc, double yc) :
      spacePoint(spp), sensorID(id), x(xx), y(yy), z(zz), xConformal(xc), yConformal(yc) {};

    /// Pointer to current SpacePoint to be used in all further steps of track finding
    const SpacePoint* spacePoint;
    /// SensorID of this hit
    VxdID   sensorID;
    /// x coordinate of this hit
    double  x;
    /// y coordinate of this hit
    double  y;
    /// z coordinate of this hit
    double  z;
    /// conformal transformed x coordinate of this hit
    double  xConformal;
    /// conformal transformed y coordinate of this hit
    double  yConformal;
  };

}
