/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDSENSORINFO_H
#define SVDSENSORINFO_H

#include <vxd/geometry/SensorInfoBase.h>

namespace Belle2 {
  namespace svd {
    class SensorInfo: public VXD::SensorInfoBase {
    public:
      SensorInfo(VxdID id = 0, float width = 0, float length = 0, float thickness = 0, int uCells = 0, int vCells = 0, float width2 = 0):
          VXD::SensorInfoBase(SensorInfo::SVD, id, width, length, thickness, uCells, vCells, width2, 0, 0) {}
      void setID(VxdID id) { m_id = id; }
    }; // class SensorInfo
  } // namespace svd
} // namespace Belle2
#endif /* SVDSENSORINFO_H */
