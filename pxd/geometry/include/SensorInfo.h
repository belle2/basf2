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
      SensorInfo(VxdID id = 0, float width = 0, float length = 0, float thickness = 0, int uCells = 0, int vCells = 0,
                 float splitLength = 0, int vCells2 = 0):
        VXD::SensorInfoBase(SensorInfo::PXD, id, width, length, thickness, uCells, vCells, 0, splitLength, vCells2), m_temperature(300),
        m_bulkDoping(0), m_backVoltage(0), m_topVoltage(0), m_sourceBorderSmallPitch(0), m_clearBorderSmallPitch(0),
        m_drainBorderSmallPitch(0), m_sourceBorderLargePitch(0), m_clearBorderLargePitch(0), m_drainBorderLargePitch(0),
        m_gateDepth(0), m_doublePixel(true), m_chargeThreshold(0), m_noiseFraction(0), m_integrationStart(0), m_integrationEnd(0)
      {
        m_hallFactor = (1.13 + 0.0008 * (m_temperature - 273));
        cook();
      }
      /** Change the SensorID, useful to copy the SensorInfo from one sensor and use it for another */
      void setID(VxdID id) { m_id = id; }

      /** Flip the Pitch segmentation along v.
       * If there are two different pixel sizes than mirror the segmentation
       * along v.
       */
      void flipVSegmentation()
      {
        if (m_splitLength <= 0) return;
        std::swap(m_vCells, m_vCells2);
        std::swap(m_sourceBorderSmallPitch, m_sourceBorderLargePitch);
        std::swap(m_clearBorderSmallPitch, m_clearBorderLargePitch);
        std::swap(m_drainBorderSmallPitch, m_drainBorderLargePitch);
        m_splitLength = (1 - m_splitLength);
        cook();
      }

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
        cook();
      }

      /** calculate constants in advance */
      void cook();

      /** Get pixel number if the given coordinate is in the
       *  correspondin internal gate trapping region or -1 otherwise
       * @param x u-coordinate in the local system
       * @param y v-coordinate in the local system
       * @return pixel number id = ix + 250*iy, or -1
       */
      int getTrappedID(double x, double y) const;

      /** Set the time window in which the sensor is active */
      void setIntegrationWindow(double start, double end)
      {
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
      /** Return the distance between the source side of the pixel and the start of the Gate for a pixel at v.
       * For these functions to work, the small pitch and large pitch values have to be correctly swapped with
       * the flipVSegmentation method. FIXME: flip... is never called! */
      double getSourceBorder(double v) const
      {
        if (v / m_length + 0.5 >= m_splitLength) return m_sourceBorderSmallPitch;
        return m_sourceBorderLargePitch;
      }
      /** Return the distance between the clear side of the pixel and the start of the Gate for a pixel at v */
      double getClearBorder(double v) const
      {
        if (v / m_length + 0.5 >= m_splitLength) return m_clearBorderSmallPitch;
        return m_clearBorderLargePitch;
      }
      /** Return the distance between the drain side of the pixel and the start of the Gate for a pixel at v */
      double getDrainBorder(double v) const
      {
        if (v / m_length + 0.5 >= m_splitLength) return m_drainBorderSmallPitch;
        return m_drainBorderLargePitch;
      }
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
      /** Return pixel kind ID
       * @param sensorID the sensor identification
       * @param v Local v coordinate
       * @return pixel kind ID in range 0..7, 0-3 for Layer=1, 4-7 for Layer=2
       */
      int getPixelKind(const VxdID sensorID, double v) const;
      /** Return pixel kind ID
       * @param sensorID the sensor identification
       * @param vID Local vcell ID
       * @return pixel kind ID in range 0, 1, 2, 3 for z55, z60, z70, z85 pixels
       */
      int getPixelKindNew(const VxdID& sensorID, int vID) const;

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
      /** The distance between the source side of the pixel and the start of the Gate, small pitch area */
      double m_sourceBorderSmallPitch;
      /** The distance between the clear side of the pixel and the start of the Gate, small pitch area  */
      double m_clearBorderSmallPitch;
      /** The distance between the drain side of the pixel and the start of the Gate, small pitch area  */
      double m_drainBorderSmallPitch;
      /** The distance between the source side of the pixel and the start of the Gate, large pitch area */
      double m_sourceBorderLargePitch;
      /** The distance between the clear side of the pixel and the start of the Gate, large pitch area  */
      double m_clearBorderLargePitch;
      /** The distance between the drain side of the pixel and the start of the Gate, large pitch area  */
      double m_drainBorderLargePitch;
      /** Return depth of the surface where the electrons will be collected */
      double m_gateDepth;
      /** True if the Sensor is a double pixel structure: every other pixel is mirrored along v */
      bool   m_doublePixel;
      /** Charge threshold */
      double m_chargeThreshold;
      /** Fixed noise fraction */
      double m_noiseFraction;

      /** The start of the integration window, the timeframe the PXD is sensitive */
      double m_integrationStart;
      /** The end of the integration window, the timeframe the PXD is sensitive */
      double m_integrationEnd;

      double m_up; /**< pixel pitch in u direction */
      double m_iup; /**< the reciprocal of the pixel pitch in u direction */

      double m_vsplit; /**< v coordinate which splits small and large pixel regions*/

      double m_vp; /**< large pixel pitch in v direction */
      double m_ivp; /**< the reciprocal of the large pixel pitch in v direction */

      double m_vp2; /**< small pixel pitch in v direction */
      double m_ivp2; /**< the reciprocal of the small pixel pitch in v direction */

      double m_hxIG; /**< size in u direction of the internal gate trapping region*/
      double m_mIGL; /**< middle of the internal gate trapping region for large pixels */
      double m_sIGL; /**< size in v direction of the internal gate trapping region for large pixels */
      double m_mIGS; /**< middle of the internal gate trapping region for small pixels */
      double m_sIGS; /**< size in v direction of the internal gate trapping region for small pixels */
    };

  }
} //Belle2 namespace
#endif
