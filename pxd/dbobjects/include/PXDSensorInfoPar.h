/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <vxd/dbobjects/VXDSensorInfoBasePar.h>

namespace Belle2 {

  /**
  * The Class for VXD geometry
  */

  class PXDSensorInfoPar: public VXDSensorInfoBasePar {

  public:
    /** Constructor which automatically sets the SensorType */

    PXDSensorInfoPar(VxdID id = 0, float width = 0, float length = 0, float thickness = 0, int uCells = 0, int vCells = 0,
                     float splitLength = 0, int vCells2 = 0):
      VXDSensorInfoBasePar(PXDSensorInfoPar::PXD, id, width, length, thickness, uCells, vCells, 0, splitLength, vCells2),
      m_bulkDoping(0), m_backVoltage(0), m_topVoltage(0), m_sourceBorderSmallPitch(0), m_clearBorderSmallPitch(0),
      m_drainBorderSmallPitch(0), m_sourceBorderLargePitch(0), m_clearBorderLargePitch(0), m_drainBorderLargePitch(0),
      m_gateDepth(0), m_doublePixel(true), m_chargeThreshold(0), m_noiseFraction(0), m_integrationStart(0), m_integrationEnd(0)
    {
    }

    /** Change the SensorID, useful to copy the SensorInfo from one sensor and use it for another */
    void setID(VxdID id) { m_id = id; }

    /** Set operation parameters like voltages */
    void setDEPFETParams(double bulkDoping, double backVoltage, double topVoltage,
                         double sourceBorderSmallPitch, double clearBorderSmallPitch, double drainBorderSmallPitch,
                         double sourceBorderLargePitch, double clearBorderLargePitch, double drainBorderLargePitch,
                         double gateDepth, bool doublePixel, double chargeThreshold, double noiseFraction)
    {
      m_bulkDoping = bulkDoping;
      m_backVoltage = backVoltage;
      m_topVoltage = topVoltage;
      m_sourceBorderSmallPitch = sourceBorderSmallPitch;
      m_clearBorderSmallPitch = clearBorderSmallPitch;
      m_drainBorderSmallPitch = drainBorderSmallPitch;
      m_sourceBorderLargePitch = sourceBorderLargePitch;
      m_clearBorderLargePitch = clearBorderLargePitch;
      m_drainBorderLargePitch = drainBorderLargePitch;
      m_gateDepth = gateDepth;
      m_doublePixel = doublePixel;
      m_chargeThreshold = chargeThreshold;
      m_noiseFraction = noiseFraction;
    }

    /** Set the time window in which the sensor is active */
    void setIntegrationWindow(double start, double end)
    {
      m_integrationStart = start;
      m_integrationEnd = end;
    }

    /** Return the bulk doping of the Silicon sensor */
    double getBulkDoping() const { return m_bulkDoping; }
    /** Return the voltage at the backside of the sensor */
    double getBackVoltage() const { return m_backVoltage; }
    /** Return the voltage at the top of the sensor */
    double getTopVoltage() const { return m_topVoltage; }
    /** Return the distance along v between the source side of a large pixel and the start of the internal gate*/
    double getSourceBorderLargePitch() const {return m_sourceBorderLargePitch;}
    /** Return the distance along v between the source side of a small pixel and the start of the internal gate*/
    double getSourceBorderSmallPitch() const {return m_sourceBorderSmallPitch;}
    /** Return the distance along u between the clear side of a large pixel and the start of the internal gate*/
    double getClearBorderLargePitch() const {return m_clearBorderLargePitch;}
    /** Return the distance along u between the clear side of a small pixel and the start of the internal gate*/
    double getClearBorderSmallPitch() const {return m_clearBorderSmallPitch;}
    /** Return the distance along v between the drain side of a large pixel and the start of the internal gate*/
    double getDrainBorderLargePitch() const {return m_drainBorderLargePitch;}
    /** Return the distance along v between the drain side of a small pixel and the start of the internal gate*/
    double getDrainBorderSmallPitch() const {return m_drainBorderSmallPitch;}
    /** Return the gate depth for the sensor */
    double getGateDepth() const { return m_gateDepth; }
    /** Return true if the Sensor is a double pixel structure: every other pixel is mirrored along v */
    bool   getDoublePixel() const { return m_doublePixel; }
    /** Get the charge threshold in ADU for the sensor. */
    double getChargeThreshold() const { return m_chargeThreshold; }
    /** Get the noise fraction for the sensor. */
    double getNoiseFraction() const { return m_noiseFraction; }
    /** Return the start of the integration window, the timeframe the PXD is sensitive */
    double getIntegrationStart() const { return m_integrationStart; }
    /** Return the end of the integration window, the timeframe the PXD is sensitive */
    double getIntegrationEnd() const { return m_integrationEnd; }

  private:

    /** Doping concentration of the silicon bulk */
    double m_bulkDoping;
    /** The voltage at the backside of the sensor */
    double m_backVoltage;
    /** The voltate at the top of the sensor */
    double m_topVoltage;
    /** The distance along v between the source side of a small pixel and the start of the internal gate*/
    double m_sourceBorderSmallPitch;
    /** The distance along u between the clear side of a small pixel and the start of the internal gate*/
    double m_clearBorderSmallPitch;
    /** The distance along v between the drain side of a small pixel and the start of the internal gate*/
    double m_drainBorderSmallPitch;
    /** The distance along v between the source side of a large pixel and the start of the internal gate*/
    double m_sourceBorderLargePitch;
    /** The distance along u between the clear side of a large pixel and the start of the internal gate*/
    double m_clearBorderLargePitch;
    /** The distance along u between the drain side of a large pixel and the start of the internal gate*/
    double m_drainBorderLargePitch;
    /** Return depth of the surface where the electrons will be collected */
    double m_gateDepth;
    /** True if the Sensor is a double pixel structure: every other pixel is mirrored along v */
    bool  m_doublePixel;
    /** Charge threshold */
    double m_chargeThreshold;
    /** Fixed noise fraction */
    double m_noiseFraction;
    /** The start of the integration window, the timeframe the PXD is sensitive */
    double m_integrationStart;
    /** The end of the integration window, the timeframe the PXD is sensitive */
    double m_integrationEnd;

    ClassDef(PXDSensorInfoPar, 6);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

