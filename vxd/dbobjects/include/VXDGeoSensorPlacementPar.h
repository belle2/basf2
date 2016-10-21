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


namespace Belle2 {

  class GearDir;

  /**
  * The Class for VXD global paramter payload
  */

  class VXDGeoSensorPlacementPar: public TObject {

  public:
    //! Default constructor
    VXDGeoSensorPlacementPar() {}
    //! Constructor using Gearbox
    //explicit VXDGeoSensorPlacementPar(const GearDir& content) { read(content); }
    //! Destructor
    ~VXDGeoSensorPlacementPar() {}
    //! Get geometry parameters from Gearbox
    //void read(const GearDir&);
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
    /** set the z position of the sensor */
    void setZ(double z)  { m_z = z; }
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
    /** flip sensor along u? */
    bool m_flipU;
    /** flip sensor along v? */
    bool m_flipV;
    /** flip sensor along w? */
    bool m_flipW;

    ClassDef(VXDGeoSensorPlacementPar, 5);  /**< ClassDef, must be the last term before the closing {}*/

  };
} // end of namespace Belle2

