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

  class GearDir;

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
      m_backplaneCapacitance(0), m_interstripCapacitance(0),
      m_couplingCapacitance(0), m_electronicNoiseU(0), m_electronicNoiseV(0)
    { }


    /** Change the SensorID. Useful to copy the SensorInfo from one sensor and use it for another.
     * @param id VxdID to be assigned to current sensor.
     */
    void setID(VxdID id) { m_id = id; }

    /** Set sensor operation parameters.
     * @param depletionVoltage Depletion voltage of the sensor.
     * @param biasVoltage Bias voltage on the sensor.
     * @param backplaneCapacitance Backplane capacitance wrt. the strips.
     * @param interstripCapacitance Interstrip capacitance for the sensor.
     * @param coupling capacitance Coupling capacitance for the strips.
     */
    void setSensorParams(double depletionVoltage, double biasVoltage,
                         double backplaneCapacitance, double interstripCapacitance, double couplingCapacitance, double electronicNoiseU,
                         double electronicNoiseV)
    {
      m_depletionVoltage = depletionVoltage;
      m_biasVoltage = biasVoltage;
      m_backplaneCapacitance = backplaneCapacitance;
      m_interstripCapacitance = interstripCapacitance;
      m_couplingCapacitance = couplingCapacitance;
      m_electronicNoiseU = electronicNoiseU;
      m_electronicNoiseV = electronicNoiseV;
    }

    /** Return the sensor temperature.*/
    double getTemperature() const {return m_temperature; }
    /** Return the depletion voltage of the sensor. */
    double getDepletionVoltage() const { return m_depletionVoltage; }
    /** Return the bias voltage on the sensor. */
    double getBiasVoltage() const { return m_biasVoltage; }
    /** Return the backplane capacitance for the sensor. */
    double getBackplaneCapacitance() const { return m_backplaneCapacitance; }
    /** Return the interstrip capacitance for the sensor. */
    double getInterstripCapacitance() const { return m_interstripCapacitance; }
    /** Return the coupling capacitance of the sensor strips */
    double getCouplingCapacitance() const { return m_couplingCapacitance; }
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
    /** The backplane capacitance wrt. the strips. */
    double m_backplaneCapacitance;
    /** The interstrip capacitance for the sensor. */
    double m_interstripCapacitance;
    /** The coupling capacitance for the sensor. */
    double m_couplingCapacitance;
    /** The electronic noise for u (short, n-side) strips. */
    double m_electronicNoiseU;
    /** The electronic noise for v (long, p-side) strips. */
    double m_electronicNoiseV;

    ClassDef(SVDSensorInfoPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

