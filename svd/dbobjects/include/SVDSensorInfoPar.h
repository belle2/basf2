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
    /** Enum for parametric access to sensor coordinates. */
    enum Coordinate {
      u = 0,
      v = 1
    };
    /**
    * Enum to flag charge carriers.
    */
    enum CarrierType {
      electron = -1, /** electrons */
      hole = +1      /** holes */
    };

    /** Constructor which automatically sets the SensorType to SensorInfo::SVD.
     * @param id VXD ID of the sensor.
     * @width Width of the sensor.
     * @length Length of the sensor.
     * @thickness Thickness of the senosr.
     * @uCells Number of strips in u dirrection.
     * @vCells Number of strips in v direction.
     * @width2 For wedge sensors, width is the width at 0, width2 is the width at maximum u.
     */
    SVDSensorInfoPar(VxdID id = 0, float width = 0, float length = 0, float thickness = 0,
                     int uCells = 0, int vCells = 0, float width2 = 0):
      VXDSensorInfoBasePar(SVDSensorInfoPar::SVD, id, width, length, thickness, uCells, vCells, width2, -1, 0),
      m_temperature(300), m_depletionVoltage(0), m_biasVoltage(0),
      m_backplaneCapacitanceU(0), m_interstripCapacitanceU(0), m_couplingCapacitanceU(0),
      m_backplaneCapacitanceV(0), m_interstripCapacitanceV(0), m_couplingCapacitanceV(0),
      m_electronicNoiseU(0), m_electronicNoiseV(0)
    { }


    /** Change the SensorID. Useful to copy the SensorInfo from one sensor and use it for another.
     * @param id VxdID to be assigned to current sensor.
     */
    void setID(VxdID id) { m_id = id; }

    /** Set sensor operation parameters.
     * @param depletionVoltage Depletion voltage of the sensor.
     * @param biasVoltage Bias voltage on the sensor.
     * @param backplaneCapacitanceU Backplane capacitance for U strips.
     * @param interstripCapacitanceU Interstrip capacitance for U strips..
     * @param coupling capacitanceU Coupling capacitance for U strips.
     * @param backplaneCapacitanceV Backplane capacitance for V strips.
     * @param interstripCapacitanceV Interstrip capacitance for V strips.
     * @param coupling capacitanceV Coupling capacitance for V strips.
     */
    void setSensorParams(double depletionVoltage, double biasVoltage,
                         double backplaneCapacitanceU, double interstripCapacitanceU,
                         double couplingCapacitanceU,
                         double backplaneCapacitanceV, double interstripCapacitanceV,
                         double couplingCapacitanceV,
                         double electronicNoiseU, double electronicNoiseV)
    {
      m_depletionVoltage = depletionVoltage;
      m_biasVoltage = biasVoltage;
      m_backplaneCapacitanceU = backplaneCapacitanceU;
      m_interstripCapacitanceU = interstripCapacitanceU;
      m_couplingCapacitanceU = couplingCapacitanceU;
      m_backplaneCapacitanceV = backplaneCapacitanceV;
      m_interstripCapacitanceV = interstripCapacitanceV;
      m_couplingCapacitanceV = couplingCapacitanceV;
      m_electronicNoiseU = electronicNoiseU;
      m_electronicNoiseV = electronicNoiseV;
    }

    /** Return the sensor temperature.*/
    double getTemperature() const {return m_temperature; }
    /** Return the depletion voltage of the sensor. */
    double getDepletionVoltage() const { return m_depletionVoltage; }
    /** Return the bias voltage on the sensor. */
    double getBiasVoltage() const { return m_biasVoltage; }
    /** Return the backplane capacitance for U-side strips. */
    double getBackplaneCapacitanceU() const { return m_backplaneCapacitanceU; }
    /** Return the interstrip capacitance for U-side strips. */
    double getInterstripCapacitanceU() const { return m_interstripCapacitanceU; }
    /** Return the coupling capacitance for U-side strips*/
    double getCouplingCapacitanceU() const { return m_couplingCapacitanceU; }
    /** Return the backplane capacitance for V-side strips. */
    double getBackplaneCapacitanceV() const { return m_backplaneCapacitanceV; }
    /** Return the interstrip capacitance for V-side strips. */
    double getInterstripCapacitanceV() const { return m_interstripCapacitanceV; }
    /** Return the coupling capacitance for V-side strips*/
    double getCouplingCapacitanceV() const { return m_couplingCapacitanceV; }
    /** Return electronic noise in e- for u (short) strips */
    double getElectronicNoiseU() const {return m_electronicNoiseU; }
    /** Return electronic noise in e- for v (long) strips */
    double getElectronicNoiseV() const {return m_electronicNoiseV; }


    /** Return Hall factor for the corresponding carrier type.
     * @param carrier electron or hole, SVDSensorInfoPar::CarrierType
     * @return The Hall factor for the actual sensor temperature.
     */
    double getHallFactor(CarrierType carrier) const
    {
      if (carrier == electron)
        return (1.13 + 0.0008 * (m_temperature - 273));
      else
        return (0.72 - 0.0005 * (m_temperature - 273));
    }


  private:
    /** Sensor temperature*/
    double m_temperature;
    /** The depletion voltage of the Silicon sensor */
    double m_depletionVoltage;
    /** The bias voltage on the sensor */
    double m_biasVoltage;
    /** The backplane capacitance for U-side strips */
    double m_backplaneCapacitanceU;
    /** The interstrip capacitance for U-side strips. */
    double m_interstripCapacitanceU;
    /** The coupling capacitance for U-side strips. */
    double m_couplingCapacitanceU;
    /** The backplane capacitance for V-side strips. */
    double m_backplaneCapacitanceV;
    /** The interstrip capacitance for V-side strips. */
    double m_interstripCapacitanceV;
    /** The coupling capacitance for V-side strips. */
    double m_couplingCapacitanceV;
    /** The electronic noise for U (short, n-side) strips. */
    double m_electronicNoiseU;
    /** The electronic noise for V (long, p-side) strips. */
    double m_electronicNoiseV;

    ClassDef(SVDSensorInfoPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

