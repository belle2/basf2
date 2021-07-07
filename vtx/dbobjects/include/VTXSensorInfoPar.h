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

  class VTXSensorInfoPar: public VXDSensorInfoBasePar {

  public:
    /** Constructor which automatically sets the SensorType */

    VTXSensorInfoPar(VxdID id = 0, float width = 0, float length = 0, float thickness = 0, int uCells = 0, int vCells = 0):
      VXDSensorInfoBasePar(VTXSensorInfoPar::VTX, id, width, length, thickness, uCells, vCells),
      m_bulkDoping(0), m_backVoltage(0), m_topVoltage(0), m_borderU(0), m_borderV(0), m_chargeThreshold(0), m_noiseFraction(0),
      m_applyElectronicEffects(true), m_electronicNoise(0), m_applyBinaryReadout(true), m_binaryHitThreshold(0),
      m_electronToADU(0), m_maxADUCode(0), m_applyPoissonSmearing(true), m_applyIntegrationWindow(true),
      m_segmentLength(0), m_electronGroupSize(0), m_electronStepTime(0), m_electronMaxSteps(0), m_hardwareDelay(0),
      m_ADCunit(0), m_cloudSize(0), m_gq(0), m_X0average(0), m_tanLorentzAngle(0), m_resolutionCoefficientU(0),
      m_resolutionCoefficientV(0), m_integrationStart(0), m_integrationEnd(0)
    {
    }

    /** Change the SensorID, useful to copy the SensorInfo from one sensor and use it for another */
    void setID(VxdID id) { m_id = id; }

    /** Set operation parameters like voltages */
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
    /** Return the distance in the U edge in the pixel where drift fields are weak and diffusion plays a role for charge transport*/
    double getBorderU() const {return m_borderU;}
    /** Return the distance in the V edge in the pixel where drift fields are weak and diffusion plays a role for charge transport*/
    double getBorderV() const {return m_borderV;}
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
    /** Return the ADC gain factor */
    double getGq() const { return m_gq; }
    /** Averaged sensors X0 (for perpendicular incidence) */
    double getX0average() const { return m_X0average; }
    /** Return the tangent of Lorentz angle */
    double getTanLorentzAngle() const { return m_tanLorentzAngle;}
    /** Return the spatial resolution coefficient for u axis */
    double getResolutionCoefficientU() const { return m_resolutionCoefficientU;}
    /** Return the spatial resolution coefficient for v axis */
    double getResolutionCoefficientV() const { return m_resolutionCoefficientV;}
    /** Return the start of the integration window, the timeframe the VTX is sensitive */
    double getIntegrationStart() const { return m_integrationStart; }
    /** Return the end of the integration window, the timeframe the VTX is sensitive */
    double getIntegrationEnd() const { return m_integrationEnd; }

  private:

    /** Doping concentration of the silicon bulk */
    double m_bulkDoping;
    /** The voltage at the backside of the sensor */
    double m_backVoltage;
    /** The voltate at the top of the sensor */
    double m_topVoltage;
    /** The distance along v between the source side of a  pixel and the start of the internal gate*/
    double m_borderU;
    /** The distance along u between the clear side of a  pixel and the start of the internal gate*/
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

    ClassDef(VTXSensorInfoPar, 8);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

