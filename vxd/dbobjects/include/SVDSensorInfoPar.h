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
#include <root/TVector3.h>

namespace Belle2 {

  class GearDir;

  /**
  * The Class for VXD geometry
  */

  class SVDSensorInfoPar: public VXDSensorInfoBasePar {

  public:

    //! Default constructor
    //SVDSensorInfoPar() {}
    //! Constructor using Gearbox
    //explicit SVDSensorInfoPar(const GearDir& content) { read(content); }
    //! Destructor
    //~SVDSensorInfoPar() {}
    //! Get geometry parameters from Gearbox
    //void read(const GearDir&);

    /** The Fano factor for silicon. */
    const double c_fanoFactorSi = 0.08;

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

    // FIXME: Consolidate with similar enums in the digitizer.
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
    /** Calculate electron mobility at a given electric field.
     * Based on C. Canali et al., IEEE, ED-22, (1975) 1045
     * @param eField Electric field, V/cm
     * @return electron mobility, cm*2/V.ns
     */
    double getElectronMobility(double E) const;
    /** Calculate hole mobility at a given electric field.
     * Based on C. Canali et al., IEEE, ED-22, (1975) 1045
     * @param eField Electric field, V/cm
     * @return hole mobility, cm*2/V.ns
     */
    double getHoleMobility(double E) const;

    /** Model of the E field inside the sensor.
     * @param point Desired position in local coordinates.
     * @return The E field vector in local coordinates.
     */
    const TVector3 getEField(const TVector3& point) const;

    /** Get B field value from the field map.
     * @param point Desired position in local coordinates.
     * @return The B field vector in local coordinates.
     */
    //const TVector3& getBField(const TVector3& point) const;

    /** Return Hall factor for the corresponding carrier type.
     * @param carrier electron or hole, SVD::SensorInfo::CarrierType
     * @return The Hall factor for the actual sensor temperature.
     */
    double getHallFactor(CarrierType carrier) const
    {
      if (carrier == electron)
        return (1.13 + 0.0008 * (m_temperature - 273));
      else
        return (0.72 - 0.0005 * (m_temperature - 273));
    }

    /** Get drift velocity for electrons or holes at a given point.
     * @param carrier Electron or hole.
     * @param point The point in local coordinates.
     * @result The vector of drift velocity in local coordinates.
     */
    //const TVector3 getVelocity(CarrierType carrier, const TVector3& point) const;

    /** Calculate Lorentz shift along a given coordinate in a magnetic field at a given position.
     * This method can only be used for a completely reconstructed 2D cluster.
     * For 1D clusters, use the following method.
     * @param u u coordinate where the shift is required
     * @param v v coordinate where the shift is required
     * @return TVector with Lorentz shift along u and v at the given position.
     */
    //const TVector3& getLorentzShift(double u, double v) const;

    /** Calculate mean Lorentz shift along a given coordinate, with B-field averaged
     * along the corresponding strip.
     * Use this for 1D clusters, where only one coordinate is known.
     * @param uCoordinate True if u, false if v.
     * @param position The position of the strip.
     * @return Mean Lorentz shift along a given coordinate.
     */
    //double getLorentzShift(bool uCoordinate, double position) const;

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

