/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once


#include <TObject.h>
#include <string>


namespace Belle2 {



  /**
  * The Class for VXD Sensor Placement payload
  */

  class VXDGeoSensorPlacementPar: public TObject {

  public:
    /** Constructor */
    VXDGeoSensorPlacementPar(int sensorID = 0, const std::string& sensorTypeID = "", double z = 0, double shift = 0,
                             double shiftR = 0, bool flipU = false, bool flipV = false, bool flipW = false):
      m_sensorID(sensorID), m_sensorTypeID(sensorTypeID), m_z(z), m_shift(shift), m_shiftR(shiftR), m_flipU(flipU), m_flipV(flipV),
      m_flipW(flipW) {}
    //! Destructor
    ~VXDGeoSensorPlacementPar() {}
    /** get the sensor id inside the ladder */
    int getSensorID() const { return m_sensorID; }
    /** set the sensor id inside the ladder */
    void setSensorID(int sensorID) { m_sensorID = sensorID; }
    /** get the sensor type id, internal id only needed for construction */
    const std::string& getSensorTypeID() const { return m_sensorTypeID; }
    /** set the sensor type id, internal id only needed for construction */
    void setSensorTypeID(const std::string&  sensorTypeID)  { m_sensorTypeID = sensorTypeID; }
    /** get the z position of the sensor */
    double getZ() const { return m_z; }
    /** get the shift of the sensor in rphi tangential direction */
    double getShift() const { return m_shift; }
    /** get the shift of the sensor in radial direction */
    double getShiftR() const { return m_shiftR; }
    /** set the z position of the sensor */
    void setZ(double z)  { m_z = z; }
    /** set the shift of the sensor in rphi tangential direction */
    void setShift(double shift)  { m_shift = shift; }
    /** set the shift of the sensor in radial direction */
    void setShiftR(double shiftR)  { m_shiftR = shiftR; }
    /** return whether or not the sensor should be flipped around the U coordinate */
    bool getFlipU() const { return m_flipU; }
    /** set whether or not the sensor should be flipped around the U coordinate */
    void setFlipU(bool flipU) { m_flipU = flipU; }
    /** return whether or not the sensor should be flipped around the V coordinate */
    bool getFlipV() const { return m_flipV; }
    /** set whether or not the sensor should be flipped around the V coordinate */
    void setFlipV(bool flipV)  { m_flipV = flipV; }
    /** return whether or not the sensor should be flipped around the W coordinate */
    bool getFlipW() const { return m_flipW; }
    /** set whether or not the sensor should be flipped around the W coordinate */
    void setFlipW(bool flipW)  { m_flipW = flipW; }

  private:

    /** ID of the sensor inside the ladder */
    int m_sensorID;
    /** Type of the sensor to be used */
    std::string m_sensorTypeID;
    /** global z position of the center of the sensor */
    double m_z;
    /** shift of the sensor relative to ladder in rphi tangential direction */
    double m_shift;
    /** shift of the sensor relative to ladder in radial direction */
    double m_shiftR;
    /** flip sensor along u? */
    bool m_flipU;
    /** flip sensor along v? */
    bool m_flipV;
    /** flip sensor along w? */
    bool m_flipW;

    ClassDef(VXDGeoSensorPlacementPar, 6);  /**< ClassDef, must be the last term before the closing {}*/

  };
} // end of namespace Belle2

