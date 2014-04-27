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

#include <vxd/geometry/SensorInfoBase.h>
#include <algorithm>

#include <root/TVector3.h>

namespace Belle2 {
  /** Namespace to encapsulate code needed for simulation and reconstrucion of the PXD */
  namespace PXD {
    /** Specific implementation of SensorInfo for PXD Sensors which provides additional pixel specific information */
    class SensorInfo: public VXD::SensorInfoBase {
    public:
      /** Constructor which automatically sets the SensorType */
      SensorInfo(VxdID id = 0, float width = 0, float length = 0, float thickness = 0, int uCells = 0, int vCells = 0, float splitLength = 0, int vCells2 = 0):
        VXD::SensorInfoBase(SensorInfo::PXD, id, width, length, thickness, uCells, vCells, 0, splitLength, vCells2), m_temperature(300),
        m_bulkDoping(0), m_backVoltage(0), m_topVoltage(0), m_sourceBorder(0), m_clearBorder(0), m_drainBorder(0),
        m_gateDepth(0), m_doublePixel(false), m_integrationStart(0), m_integrationEnd(0) {
        m_hallFactor = (1.13 + 0.0008 * (m_temperature - 273));
      }
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

      /** Return the temperature of the sensor */
      double getTemperature() const {return m_temperature;}
      /** Return the bulk doping of the Silicon sensor */
      double getBulkDoping() const { return m_bulkDoping; }
      /** Return the voltage at the backside of the sensor */
      double getBackVoltage() const { return m_backVoltage; }
      /** Return the voltage at the top of the sensor */
      double getTopVoltage() const { return m_topVoltage; }
      /** Return the distance between the source side of the pixel and the start of the Gate */
      double getSourceBorder() const { return m_sourceBorder; }
      /** Return the distance between the clear side of the pixel and the start of the Gate */
      double getClearBorder() const { return m_clearBorder; }
      /** Return the distance between the drain side of the pixel and the start of the Gate */
      double getDrainBorder() const { return m_drainBorder; }
      /** Return depth of the surface where the electrons will be collected */
      double getGateDepth() const { return m_gateDepth; }
      /** Return true if the Sensor is a double pixel structure: every other pixel is mirrored along v */
      bool   getDoublePixel() const { return m_doublePixel; }

      /** Return the start of the integration window, the timeframe the PXD is sensitive */
      double getIntegrationStart() const { return m_integrationStart; }
      /** Return the end of the integration window, the timeframe the PXD is sensitive */
      double getIntegrationEnd() const { return m_integrationEnd; }
      /** Return the Hall factor for electrons at sensor temperature.*/
      double getHallFactor() const { return m_hallFactor; }
      /** Calculate electron mobility at a given electric field.
       * Based on C. Canali et al., IEEE, ED-22, (1975) 1045
       * @param eField Electric field, V/cm
       * @return electron mobility, cm*2/V.ns
       */
      double getElectronMobility(double E) const;
      /** Model of the E field inside the sensor.
       * @param point Desired position in local coordinates.
       * @param info The SensorInfo for the current sensor.
       * @return The E field vector in local coordinates.
       */
      const TVector3 getEField(const TVector3& point) const;
      /** Get B field value from the field map.
       * @param point Desired position in local coordinates.
       * @param info the SensorInfo for the current sensor.
       * @return The B field vector in local coordinates.
       */
      const TVector3 getBField(const TVector3& point) const;
      /** Calculate Lorentz shift.
       * @param u Local u coordinate
       * @param v Local v coordinate
       * @return 3-vector (du, dv, 0) of Lorentz shifts.
       */
      const TVector3 getLorentzShift(double u, double v) const;
      /** Calculate drift velocity of an electron.
       * @param E Electric field vector
       * @param B Magnetic field vector
       * @result drift velocity of an electron in the E+B field.
       */
      const TVector3 getDriftVelocity(const TVector3& E, const TVector3& B) const;

    protected:
      /** Calculate Lorentz shift factor.
       * This factor is constant for a sensor; the actual, position-dependent
       * Lorentz shift is factor cross B.*/
      void setLorentzFactor();

      /** The temperature of the sensor */
      double m_temperature;
      /** The bulk doping of the Silicon sensor */
      double m_hallFactor;
      /** Doping concentration of the silicon bulk */
      double m_bulkDoping;
      /** The voltage at the backside of the sensor */
      double m_backVoltage;
      /** The voltate at the top of the sensor */
      double m_topVoltage;
      /** The distance between the source side of the pixel and the start of the Gate */
      double m_sourceBorder;
      /** The distance between the clear side of the pixel and the start of the Gate */
      double m_clearBorder;
      /** The distance between the drain side of the pixel and the start of the Gate */
      double m_drainBorder;
      /** Return depth of the surface where the electrons will be collected */
      double m_gateDepth;
      /** True if the Sensor is a double pixel structure: every other pixel is mirrored along v */
      bool   m_doublePixel;

      /** The start of the integration window, the timeframe the PXD is sensitive */
      double m_integrationStart;
      /** The end of the integration window, the timeframe the PXD is sensitive */
      double m_integrationEnd;
      /** Lorentz factor to calculate Lorentz shifts. */
      TVector3 m_lorentzFactor;
    };

  }
} //Belle2 namespace
#endif
