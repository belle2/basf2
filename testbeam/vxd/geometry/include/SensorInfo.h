/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Tadeas Bilka                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TEL_SENSORINFO_H
#define TEL_SENSORINFO_H

#include <vxd/geometry/SensorInfoBase.h>
#include <algorithm>

namespace Belle2 {
  /** Namespace to encapsulate code needed for simulation and reconstruction of the PXD testbeam */
  namespace TB {
    /** Specific implementation of SensorInfo for telescope ensors which provides additional specific information.
     * This implementation is mostly meant for EuTel MAPS sensors.
     */
    class SensorInfo: public VXD::SensorInfoBase {
    public:
      /** Constructor which automatically sets the SensorType */
      SensorInfo(VxdID id = 0, float width = 0, float length = 0, float thickness = 0, int uCells = 0, int vCells = 0, float splitLength = 0, int vCells2 = 0):
        VXD::SensorInfoBase(SensorInfo::TEL, id, width, length, thickness, uCells, vCells, 0, splitLength, vCells2) {}
      /** Change the SensorID, useful to copy the SensorInfo from one sensor and use it for another */
      void setID(VxdID id) { m_id = id; }

      /** Set the time window in which the sensor is active */
      void setIntegrationWindow(double start, double end) {
        m_integrationStart = start;
        m_integrationEnd = end;
      }

      /** Return the start of the integration window, the timeframe the telescope is sensitive */
      double getIntegrationStart() const { return m_integrationStart; }
      /** Return the end of the integration window, the timeframe the telescope is sensitive */
      double getIntegrationEnd() const { return m_integrationEnd; }

    protected:
      /** The start of the integration window, the timeframe the telescope is sensitive */
      double m_integrationStart;
      /** The end of the integration window, the timeframe the telescope is sensitive */
      double m_integrationEnd;
    };

  }
} //Belle2 namespace
#endif
