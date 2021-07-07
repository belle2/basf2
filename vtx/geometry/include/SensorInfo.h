/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef VTX_SENSORINFO_H
#define VTX_SENSORINFO_H

#include <vxd/geometry/SensorInfoBase.h>
#include <algorithm>

#include <root/TVector3.h>

namespace Belle2 {
  /** Namespace to encapsulate code needed for simulation and reconstrucion of the VTX */
  namespace VTX {
    /** Specific implementation of SensorInfo for VTX Sensors which provides additional pixel specific information */
    class SensorInfo: public VXD::SensorInfoBase {
    public:
      /** Constructor which automatically sets the SensorType */
      SensorInfo(VxdID id = 0, float width = 0, float length = 0, float thickness = 0, int uCells = 0, int vCells = 0):
        VXD::SensorInfoBase(SensorInfo::VTX, id, width, length, thickness, uCells, vCells, 0), m_temperature(300),
        m_hallFactor(0), m_bulkDoping(0), m_backVoltage(0), m_topVoltage(0), m_borderU(0), m_borderV(0), m_chargeThreshold(0),
        m_noiseFraction(0),
        m_applyElectronicEffects(true), m_electronicNoise(0), m_applyBinaryReadout(true), m_binaryHitThreshold(0),
        m_electronToADU(0), m_maxADUCode(0), m_applyPoissonSmearing(true), m_applyIntegrationWindow(true),
        m_segmentLength(0), m_electronGroupSize(0), m_electronStepTime(0), m_electronMaxSteps(0), m_hardwareDelay(0),
        m_ADCunit(0), m_cloudSize(0), m_gq(0), m_X0average(0), m_tanLorentzAngle(0), m_resolutionCoefficientU(0),
        m_resolutionCoefficientV(0),
        m_integrationStart(0), m_integrationEnd(0)
      {
        m_hallFactor = (1.13 + 0.0008 * (m_temperature - 273));
        cook();
      }
      /** Change the SensorID, useful to copy the SensorInfo from one sensor and use it for another */
      void setID(VxdID id) { m_id = id; }

      /** Set VTX sensor parameters */
      void setVTXSensorParams(double bulkDoping, double backVoltage, double topVoltage,
                              double borderU, double borderV, double chargeThreshold, double noiseFraction,
                              bool applyElectronicEffects, double electronicNoise, bool applyBinaryReadout,
                              double binaryHitThreshold, double electronToADU, double maxADUCode,
                              bool applyPoissonSmearing, bool applyIntegrationWindow, double segmentLength,
                              double electronGroupSize, double electronStepTime, double electronMaxSteps,
                              double hardwareDelay, double ADCunit, double cloudSize, double gq, double X0average,
                              double tanLorentzAngle, double resolutionCoefficientU, double resolutionCoefficientV)
      {
        m_bulkDoping = bulkDoping;
        m_backVoltage = backVoltage;
        m_topVoltage = topVoltage;
        m_borderU = borderU;
        m_borderV = borderV;
        m_chargeThreshold = chargeThreshold;
        m_noiseFraction = noiseFraction;
        m_applyElectronicEffects = applyElectronicEffects;
        m_electronicNoise = electronicNoise;
        m_applyBinaryReadout = applyBinaryReadout;
        m_binaryHitThreshold = binaryHitThreshold;
        m_electronToADU = electronToADU;
        m_maxADUCode = maxADUCode;
        m_applyPoissonSmearing = applyPoissonSmearing;
        m_applyIntegrationWindow = applyIntegrationWindow;
        m_segmentLength = segmentLength;
        m_electronGroupSize = electronGroupSize;
        m_electronStepTime = electronStepTime;
        m_electronMaxSteps = electronMaxSteps;
        m_hardwareDelay = hardwareDelay;
        m_ADCunit = ADCunit;
        m_cloudSize = cloudSize;
        m_gq = gq;
        m_X0average = X0average;
        m_tanLorentzAngle = tanLorentzAngle;
        m_resolutionCoefficientU = resolutionCoefficientU;
        m_resolutionCoefficientV = resolutionCoefficientV;
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
      /** Get the charge threshold in ADU for the sensor. */
      double getChargeThreshold() const { return m_chargeThreshold; }
      /** Get the noise fraction for the sensor. */
      double getNoiseFraction() const { return m_noiseFraction; }
      /** Return true if electronic effects are applied */
      bool getApplyElectronicEffects() const { return m_applyElectronicEffects; }
      /** Return the noise added by the electronics, set in ENC*/
      double getElectronicNoise() const { return m_electronicNoise; }
      /** Return true if binary readout is applied */
      bool getApplyBinaryReadout() const { return m_applyBinaryReadout; }
      /** Return the binary hit treshold, set in ENC */
      double getBinaryHitThreshold() const { return m_binaryHitThreshold; }
      /** Return the ENC equivalent of 1 ADU */
      double getElectronToADU() const { return m_electronToADU; }
      /** Return the maximum code for analog to digital converter (clamping) */
      double getMaxADUCode() const { return m_maxADUCode; }
      /** Return true if Poisson smearing of electrons collected on pixels is applied */
      bool getApplyPoissonSmearing() const { return m_applyPoissonSmearing; }
      /** Return true if integration window is used*/
      bool getApplyIntegrationWindow() const { return m_applyIntegrationWindow; }
      /** Return the maximum segment length (in mm) */
      double getSegmentLength() const { return m_segmentLength; }
      /** Return N : Split Signalpoints in smaller groups of N electrons (in e) */
      double getElectronGroupSize() const { return m_electronGroupSize; }
      /** Return the time step for tracking electron groups in readout plane (in ns) */
      double getElectronStepTime() const { return m_electronStepTime; }
      /** Return the maximum number of steps when propagating electrons */
      double getElectronMaxSteps() const { return m_electronMaxSteps; }
      /** Return the constant time delay between bunch crossing and switching on triggergate (in ns) */
      double getHardwareDelay() const { return m_hardwareDelay; }
      /** Return the ADC conversion factor */
      double getADCunit() const { return m_ADCunit; }
      /** Return the diffusion coefficient */
      double getCloudSize() const { return m_cloudSize; }
      /** Return the tangent of Lorentz angle */
      double getTanLorentzAngle() const { return m_tanLorentzAngle;}
      /** Return the spatial resolution coefficient for u axis */
      double getResolutionCoefficientU() const { return m_resolutionCoefficientU;}
      /** Return the spatial resolution coefficient for v axis */
      double getResolutionCoefficientV() const { return m_resolutionCoefficientV;}
      /** Return the ADC gain factor */
      double getGq() const { return m_gq; }
      /** Averaged sensors X0 (for perpendicular incidence) */
      double getX0average() const { return m_X0average; }
      /** Return the start of the integration window, the timeframe the VTX is sensitive */
      double getIntegrationStart() const { return m_integrationStart; }
      /** Return the end of the integration window, the timeframe the VTX is sensitive */
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
       * @return pixel kind ID in range 0
       */
      int getPixelKind(const VxdID sensorID, double v) const;
      /** Return pixel kind ID
       * @param sensorID the sensor identification
       * @param vID Local vcell ID
       * @return pixel kind ID in range 0
       */
      int getPixelKindNew(const VxdID& sensorID, int vID) const;

    protected:
      /** Calculate Lorentz shift factor.
       * This factor is constant for a sensor; the actual, position-dependent
       * Lorentz shift is factor cross B.*/
      void setLorentzFactor();

      /** The temperature of the sensor */
      double m_temperature;
      /** The Hall factor of the Silicon sensor */
      double m_hallFactor;
      /** Doping concentration of the silicon bulk */
      double m_bulkDoping;
      /** The voltage at the backside of the sensor */
      double m_backVoltage;
      /** The voltate at the top of the sensor */
      double m_topVoltage;
      /** The distance in the U edge in the pixel where drift fields are weak and diffusion plays a role for charge transport*/
      double m_borderU;
      /** The distance in the U edge in the pixel where drift fields are weak and diffusion plays a role for charge transport*/
      double m_borderV;
      /** Charge threshold */
      double m_chargeThreshold;
      /** Fixed noise fraction */
      double m_noiseFraction;
      /** Whether or not to apply noise */
      bool m_applyElectronicEffects;
      /** Amount of noise to apply, set in ENC */
      double m_electronicNoise;
      /** Whether or not to apply binary readout */
      bool m_applyBinaryReadout;
      /** Hit threshold for binary readout in ENC */
      double m_binaryHitThreshold;
      /** ENC equivalent of 1 ADU */
      double m_electronToADU;
      /** Maximum code for analog to digital converter (clamping) */
      double m_maxADUCode;
      /** Whether or not to apply poission fluctuation of charge */
      bool m_applyPoissonSmearing;
      /** Whether or not to apply a time window cut */
      bool m_applyIntegrationWindow;
      /** Max. Segment length to use for charge drifting (in mm) */
      double m_segmentLength;
      /** Max number of electrons per random walk */
      double m_electronGroupSize;
      /** Timeframe for one random walk step (in ns) */
      double m_electronStepTime;
      /** Maximum number of random walks before abort */
      double m_electronMaxSteps;
      /** Hardware delay between time of bunch crossing and switching on triggergate in ns */
      double m_hardwareDelay;
      /** ADC conversion factor */
      double m_ADCunit;
      /** Diffusion coefficient */
      double m_cloudSize;
      /** ADC gain factor */
      double m_gq;
      /** Averaged sensors X0 (for perpendicular incidence) */
      double m_X0average;
      /** Tangent of Lorentz angle */
      double m_tanLorentzAngle;
      /** Spatial resolution coefficient for u axis*/
      double m_resolutionCoefficientU;
      /** Spatial resolution coefficient for v axis*/
      double m_resolutionCoefficientV;

      /** The start of the integration window, the timeframe the VTX is sensitive */
      double m_integrationStart;
      /** The end of the integration window, the timeframe the VTX is sensitive */
      double m_integrationEnd;

      // Note : Not usefull for VTX?
      // double m_up; /**< pixel pitch in u direction */
      // double m_iup; /**< the reciprocal of the pixel pitch in u direction */

      // double m_vsplit; /**< v coordinate which splits small and large pixel regions*/

      // double m_vp; /**< large pixel pitch in v direction */
      // double m_ivp; /**< the reciprocal of the large pixel pitch in v direction */

      // double m_vp2; /**< small pixel pitch in v direction */
      // double m_ivp2; /**< the reciprocal of the small pixel pitch in v direction */

      // double m_hxIG; /**< size in u direction of the internal gate trapping region*/
      // double m_mIGL; /**< middle of the internal gate trapping region for large pixels */
      // double m_sIGL; /**< size in v direction of the internal gate trapping region for large pixels */
      // double m_mIGS; /**< middle of the internal gate trapping region for small pixels */
      // double m_sIGS; /**< size in v direction of the internal gate trapping region for small pixels */
    };

  }
} //Belle2 namespace
#endif
