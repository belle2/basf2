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
  /** Namespace to encapsulate code needed for simulation and reconstrucion of the PXD */
  namespace PXD {
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

      /** Set operation parameters like voltages */
      void setDEPFETParams(double bulkDoping, double backVoltage, double topVoltage, double sourceBorder, double clearBorder, double drainBorder, double gateDepth, bool doublePixel) {
        m_bulkDoping = bulkDoping;
        m_backVoltage = backVoltage;
        m_topVoltage = topVoltage;
        m_sourceBorder = sourceBorder;
        m_clearBorder = clearBorder;
        m_drainBorder = drainBorder;
        m_gateDepth = gateDepth;
        m_doublePixel = doublePixel;
      }

      /** Set the time window in which the sensor is active */
      void setIntegrationWindow(double start, double end) {
        m_integrationStart = start;
        m_integrationEnd = end;
      }

      double getBulkDoping() const { return m_bulkDoping; }
      double getBackVoltage() const { return m_backVoltage; }
      double getTopVoltage() const { return m_topVoltage; }
      double getSourceBorder() const { return m_sourceBorder; }
      double getClearBorder() const { return m_clearBorder; }
      double getDrainBorder() const { return m_drainBorder; }
      double getGateDepth() const { return m_gateDepth; }
      bool   getDoublePixel() const { return m_doublePixel; }

      double getIntegrationStart() const { return m_integrationStart; }
      double getIntegrationEnd() const { return m_integrationEnd; }
    protected:
      double m_bulkDoping;
      double m_backVoltage;
      double m_topVoltage;
      double m_sourceBorder;
      double m_clearBorder;
      double m_drainBorder;
      double m_gateDepth;
      bool   m_doublePixel;

      double m_integrationStart;
      double m_integrationEnd;
    };

  }
} //Belle2 namespace
#endif
