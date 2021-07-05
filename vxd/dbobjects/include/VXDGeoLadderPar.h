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
#include <vector>

#include <vxd/dbobjects/VXDGeoSensorPlacementPar.h>


namespace Belle2 {



  /**
  * The Class for VXD Ladder payload
  */
  class VXDGeoLadderPar: public TObject {
  public:
    /** Constructor */
    VXDGeoLadderPar(int layerID = 0, double shift = 0, double radius = 0,
                    double slantedAngle = 0, double slantedRadius = 0,
                    double glueSize = -1, const std::string& glueMaterial = ""):
      m_layerID(layerID), m_shift(shift), m_radius(radius),
      m_slantedAngle(slantedAngle), m_slantedRadius(slantedRadius),
      m_glueSize(glueSize), m_glueMaterial(glueMaterial) {}
    //! Destructor
    ~VXDGeoLadderPar() {}

    /** get the layer id */
    int getLayerID() const { return m_layerID; }
    /** set the layer id */
    void setLayerID(int layerID)  { m_layerID = layerID; }
    /** get the shift along the u coordinate for all sensors in the ladder */
    double getShift() const { return m_shift; }
    /** set the shift along the u coordinate for all sensors in the ladder */
    void setShift(double shift)  { m_shift = shift; }
    /** get the radius of all sensors except slanted ones */
    double getRadius() const { return m_radius; }
    /** set the radius of all sensors except slanted ones */
    void setRadius(double radius)  { m_radius = radius; }
    /** get the slant angle for slanted sensors */
    double getSlantedAngle() const { return m_slantedAngle; }
    /** set the slant angle for slanted sensors */
    void setSlantedAngle(double slantedAngle) { m_slantedAngle = slantedAngle; }
    /** get the radius for slanted sensors */
    double getSlantedRadius() const { return m_slantedRadius; }
    /** set the radius for slanted sensors */
    void setSlantedRadius(double slantedRadius)  { m_slantedRadius = slantedRadius; }
    /** get the additional glue size, e.g. the amount the glue is produding on all sides of the joint */
    double getGlueSize() const { return m_glueSize; }
    /** set the additional glue size, e.g. the amount the glue is produding on all sides of the joint */
    void setGlueSize(double glueSize)  { m_glueSize = glueSize; }
    /** get the glue material */
    const std::string& getGlueMaterial() const { return m_glueMaterial; }
    /** set the glue material */
    void setGlueMaterial(const std::string& glueMaterial) { m_glueMaterial = glueMaterial; }
    /** get list of sensors */
    const std::vector<VXDGeoSensorPlacementPar>& getSensors() const { return m_sensors; }
    /** add a sensor to the list of sensors in the ladder */
    void addSensor(const VXDGeoSensorPlacementPar& sensor) { m_sensors.push_back(sensor); }
  private:

    /** ID of the ladder inside the layer */
    int m_layerID;
    /** Shift of the ladder along local u */
    double m_shift;
    /** Radius where to place the ladder */
    double m_radius;
    /** Angle of the module with respect to the global z-axis for slanted sensors */
    double m_slantedAngle;
    /** Radius of the center of the active area for slanted sensors */
    double m_slantedRadius;
    /** Oversize of the glue between the ladders, <0 means no glue */
    double m_glueSize;
    /** name of the Material used as glue */
    std::string m_glueMaterial;
    /** list of all sensors to be placed in the ladder */
    std::vector<VXDGeoSensorPlacementPar> m_sensors;

    ClassDef(VXDGeoLadderPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

