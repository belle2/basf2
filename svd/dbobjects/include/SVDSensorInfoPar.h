/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once


#include <TObject.h>
#include <string>

#include <vxd/dbobjects/VXDSensorInfoBasePar.h>

namespace Belle2 {


  /**
  * The Class for VXD geometry
  */

  class SVDSensorInfoPar: public VXDSensorInfoBasePar {

  public:
    /** Constructor which automatically sets the SensorType to SensorInfo::SVD.
     * @param id VXD ID of the sensor.
     * @param width Width of the sensor.
     * @param length Length of the sensor.
     * @param thickness Thickness of the senosr.
     * @param uCells Number of strips in u dirrection.
     * @param vCells Number of strips in v direction.
     * @param width2 For wedge sensors, width is the width at 0, width2 is the width at maximum u.
     */
    SVDSensorInfoPar(VxdID id = 0, float width = 0, float length = 0, float thickness = 0,
                     int uCells = 0, int vCells = 0, float width2 = 0):
      VXDSensorInfoBasePar(SVDSensorInfoPar::SVD, id, width, length, thickness, uCells, vCells,
                           width2, -1, 0),
      m_temperature(300), m_stripEdgeU(0), m_stripEdgeV(0),
      m_depletionVoltage(0), m_biasVoltage(0),
      m_backplaneCapacitanceU(0), m_interstripCapacitanceU(0), m_couplingCapacitanceU(0),
      m_backplaneCapacitanceV(0), m_interstripCapacitanceV(0), m_couplingCapacitanceV(0),
      m_aduEquivalentU(0), m_aduEquivalentV(0),
      m_electronicNoiseU(0), m_electronicNoiseV(0),
      m_aduEquivalentSbwU(0), m_aduEquivalentSbwV(0),
      m_electronicNoiseSbwU(0), m_electronicNoiseSbwV(0)
    {}

    /** Change the SensorID. Useful to copy the SensorInfo from one sensor and use it
     * for another.
     * @param id VxdID to be assigned to current sensor.
     */
    void setID(VxdID id) { m_id = id; }

    /** Set sensor operation parameters.
     * @param stripEdgeU Distance from end of strip to edge of active area.
     * @param stripEdgeV Distance from end of strip to edge of active area.
     * @param depletionVoltage Depletion voltage of the sensor.
     * @param biasVoltage Bias voltage on the sensor.
     * @param backplaneCapacitanceU Backplane capacitance/cm for U strips.
     * @param interstripCapacitanceU Interstrip capacitance/cm for U strips..
     * @param coupling capacitanceU Coupling capacitance/cm for U strips.
     * @param backplaneCapacitanceV Backplane capacitance/cm for V strips.
     * @param interstripCapacitanceV Interstrip capacitance/cm for V strips.
     * @param coupling capacitanceV Coupling capacitance/cm for V strips,
     * @param AduEquivalentU Charge in electrons per 1 ADU, U strips.
     * @param AduEquivalentV Charge in electrons per 1 ADU, V strips.
     * @param electronicNoiseU Noise on U-strips; for barrels the value for Origami,
     * @param electronicNoiseV Noise on V-strips, for barrels the value for Origami,
     * @param AduEquivalentSbwU Charge in electrons per 1 ADU, u-strips in barrel Sbw sensors.
     * @param AduEquivalentSbwV Charge in electrons per 1 ADU, v-strips in barrel Sbw sensors.
     * @param electronicNoiseSbwU Noise on U strips of backward barrel senosrs,
     * @param electronicNoiseSbwV Noise on V strips of backward barrel sensors
     */
    void setSensorParams(double stripEdgeU, double stripEdgeV,
                         double depletionVoltage, double biasVoltage,
                         double backplaneCapacitanceU, double interstripCapacitanceU,
                         double couplingCapacitanceU,
                         double backplaneCapacitanceV, double interstripCapacitanceV,
                         double couplingCapacitanceV,
                         double AduEquivalentU, double AduEquivalentV,
                         double electronicNoiseU, double electronicNoiseV,
                         double AduEquivalentSbwU, double AduEquivalentSbwV,
                         double electronicNoiseSbwU, double electronicNoiseSbwV)
    {
      m_stripEdgeU = stripEdgeU,
      m_stripEdgeV = stripEdgeV,
      m_depletionVoltage = depletionVoltage;
      m_biasVoltage = biasVoltage;
      m_backplaneCapacitanceU = backplaneCapacitanceU;
      m_interstripCapacitanceU = interstripCapacitanceU;
      m_couplingCapacitanceU = couplingCapacitanceU;
      m_backplaneCapacitanceV = backplaneCapacitanceV;
      m_interstripCapacitanceV = interstripCapacitanceV;
      m_couplingCapacitanceV = couplingCapacitanceV;
      m_aduEquivalentU = AduEquivalentU;
      m_aduEquivalentV = AduEquivalentV;
      m_electronicNoiseU = electronicNoiseU;
      m_electronicNoiseV = electronicNoiseV;
      m_aduEquivalentSbwU = AduEquivalentSbwU;
      m_aduEquivalentSbwV = AduEquivalentSbwV;
      m_electronicNoiseSbwU = electronicNoiseSbwU;
      m_electronicNoiseSbwV = electronicNoiseSbwV;
    }

    /** Return the sensor temperature.*/
    double getTemperature() const {return m_temperature; }
    /** Return the distance between end of strip and edge of active area.*/
    double getStripEdgeU() const {return m_stripEdgeU; }
    /** Return the distance between end of strip and edge of active area.*/
    double getStripEdgeV() const {return m_stripEdgeV; }
    /** Return the depletion voltage of the sensor. */
    double getDepletionVoltage() const { return m_depletionVoltage; }
    /** Return the bias voltage on the sensor. */
    double getBiasVoltage() const { return m_biasVoltage; }
    /** Return the backplane capacitance/cm for U-side strips. */
    double getBackplaneCapacitanceU() const { return m_backplaneCapacitanceU; }
    /** Return the interstrip capacitance/cm for U-side strips. */
    double getInterstripCapacitanceU() const { return m_interstripCapacitanceU; }
    /** Return the coupling capacitance/cm for U-side strips*/
    double getCouplingCapacitanceU() const { return m_couplingCapacitanceU; }
    /** Return the backplane capacitance/cm for V-side strips. */
    double getBackplaneCapacitanceV() const { return m_backplaneCapacitanceV; }
    /** Return the interstrip capacitance/cm for V-side strips. */
    double getInterstripCapacitanceV() const { return m_interstripCapacitanceV; }
    /** Return the coupling capacitance/cm for V-side strips*/
    double getCouplingCapacitanceV() const { return m_couplingCapacitanceV; }
    /** Return ADU equivalent for U strips */
    double getAduEquivalentU() const {return m_aduEquivalentU;}
    /** Return ADU equivalent for V strips */
    double getAduEquivalentV() const {return m_aduEquivalentV; }
    /** Return electronic noise in e- for u strips */
    double getElectronicNoiseU() const {return m_electronicNoiseU; }
    /** Return electronic noise in e- for v strips */
    double getElectronicNoiseV() const {return m_electronicNoiseV; }
    /** Return ADU equivalent for U strips in Sbw barrel sensor */
    double getAduEquivalentSbwU() const {return m_aduEquivalentSbwU;}
    /** Return ADU equivalent for V strips in Sbw barrel sensor */
    double getAduEquivalentSbwV() const {return m_aduEquivalentSbwV; }
    /** Return electronic noise in e- for u strips in bw barrel sensors */
    double getElectronicNoiseSbwU() const {return m_electronicNoiseSbwU; }
    /** Return electronic noise in e- for v strips in bw barrel sensors */
    double getElectronicNoiseSbwV() const {return m_electronicNoiseSbwV; }

  private:
    /** Sensor temperature*/
    double m_temperature;
    /** The distance between end of strips and edge of active area */
    double m_stripEdgeU;
    /** The distance between end of strips and edge of active area */
    double m_stripEdgeV;
    /** The depletion voltage of the Silicon sensor */
    double m_depletionVoltage;
    /** The bias voltage on the sensor */
    double m_biasVoltage;
    /** The backplane capacitance/cm for U-side strips */
    double m_backplaneCapacitanceU;
    /** The interstrip capacitance/cm for U-side strips. */
    double m_interstripCapacitanceU;
    /** The coupling capacitance/cm for U-side strips. */
    double m_couplingCapacitanceU;
    /** The backplane capacitance/cm for V-side strips. */
    double m_backplaneCapacitanceV;
    /** The interstrip capacitance/cm for V-side strips. */
    double m_interstripCapacitanceV;
    /** The coupling capacitance/cm for V-side strips. */
    double m_couplingCapacitanceV;
    /** ADU equivalent (electrons/ADU) for U strips */
    double m_aduEquivalentU;
    /** ADU equivalent (electrons/ADU) for V strips */
    double m_aduEquivalentV;
    /** The electronic noise for U strips. */
    double m_electronicNoiseU;
    /** The electronic noise for V strips. */
    double m_electronicNoiseV;
    /** ADU equivalent (electrons/ADU) for U strips, Sbw barrel sensors */
    double m_aduEquivalentSbwU;
    /** ADU equivalent (electrons/ADU) for V strips, Sbw barrel sensors */
    double m_aduEquivalentSbwV;
    /** The electronic noise for U strips in bw barrel (non-Origami) sensors. */
    double m_electronicNoiseSbwU;
    /** The electronic noise for V strips in bw barrel (non-Origami) sensors. */
    double m_electronicNoiseSbwV;

    ClassDef(SVDSensorInfoPar, 7);
  };
} // end of namespace Belle2

