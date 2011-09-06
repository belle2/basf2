/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXD_SENSORINFO_H
#define PXD_SENSORINFO_H

#include <pxd/vxd/SensorInfoBase.h>
#include <algorithm>

namespace Belle2 {
  namespace pxd {
    /** Specific implementation of SensorInfo for PXD Sensors which provides additional pixel specific information */
    class SensorInfo: public VXD::SensorInfoBase {
    public:
      /** Constructor which automatically sets the SensorType */
      SensorInfo(VxdID id = 0, float width = 0, float length = 0, float thickness = 0, int uCells = 0, int vCells = 0, float splitLength = 0, int vCells2 = 0):
          VXD::SensorInfoBase(SensorInfo::PXD, id, width, length, thickness, uCells, vCells, 0, splitLength, vCells2) {}
      /** Change the SensorID, useful to copy the SensorInfo from one sensor and use it for another */
      void setID(VxdID id) { m_id = id; }
      /** Flip the Pitch segmentation along v.
       * If there are two different pixel sizes than mirror the segmentation
       * along v.
       */
      void flipVSegmentation() {
        if (m_splitLength <= 0) return;
        std::swap(m_vCells, m_vCells2);
        m_splitLength = (1 - m_splitLength);
      }
    };
  }
} //Belle2 namespace
#endif
