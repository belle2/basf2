/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef GEOVXDCOMPONENTS_H
#define GEOVXDCOMPONENTS_H

#include <vxd/geometry/SensorInfoBase.h>

#include <string>
#include <vector>

class G4LogicalVolume;

namespace Belle2 {
  /** Class holding all parameters to place a VXD geometry subcomponent */
  class VXDGeoPlacement {
  public:
    /** Enum describing where to place the component vertically */
    enum EPosW {
      c_below,  /**< Place the component below the mother */
      c_bottom, /**< Place the component at the bottom of the mother */
      c_center, /**< Place the component at the center of the mother */
      c_top,    /**< Place the component at the top of the mother */
      c_above   /**< Place the component above the mother */
    };
    /** Constructor */
    VXDGeoPlacement(const std::string& name = "", double u = 0, double v = 0,
                    std::string w = "bottom", double woffset = 0);
    /** get Name of the component */
    const std::string& getName() const { return m_name; }
    /** get local u coordinate where to place the component */
    double getU() const { return m_u; }
    /** get local v coordinate where to place the component */
    double getV() const { return m_v; }
    /** get local w position where to place the component */
    EPosW getW() const { return m_w; }
    /** get offset to local w position where to place the component */
    double getWOffset() const { return m_woffset; }
  private:
    /** Name of the component */
    std::string m_name;
    /** u coordinate where to place the component */
    double m_u;
    /** v coordinate where to place the component */
    double m_v;
    /** w coordinate where to place the component */
    EPosW m_w;
    /** Offset to the w placement of the component */
    double m_woffset;
  };

  /** Class holding all parameters for an VXD geometry component */
  class VXDGeoComponent {
  public:
    /** Constructor */
    VXDGeoComponent(const std::string& material = "", const std::string& color = "",
                    double width = 0, double width2 = 0, double length = 0, double height = 0):
      m_volume(0), m_material(material), m_color(color), m_width(width), m_width2(width2), m_length(length),
      m_height(height) {}
    /** get the pointer to the logical volume, NULL if not yet created */
    G4LogicalVolume* getVolume() const { return m_volume; }
    /** set the pointer to the logical volume */
    void setVolume(G4LogicalVolume* volume) { m_volume = volume; }
    /** get the name of the Material for the component */
    const std::string& getMaterial() const { return m_material; }
    /** get the name of the color for the component */
    const std::string& getColor() const { return m_color; }
    /** get the width of the component */
    double getWidth() const { return m_width; }
    /** set the width of the component. This will automatically set width2 to zero */
    void setWidth(double width) { m_width = width; m_width2 = 0; }
    /** get the forward width of the component, 0 for rectangular */
    double getWidth2() const { return m_width2; }
    /** get the length of the component */
    double getLength() const { return m_length; }
    /** set the length of the component */
    void setLength(double length) { m_length = length; }
    /** get the height of the component */
    double& getHeight() { return m_height; }
    /** get the height of the component */
    double getHeight() const { return m_height; }
  private:
    /** Pointer to the Logical volume if already created  */
    G4LogicalVolume* m_volume; //! transient member, owned by Geant4
    /** Name of the material of the component */
    std::string m_material;
    /** Name of the color of the component */
    std::string m_color;
    /** width of the component */
    double m_width;
    /** forward width of the component, 0 for recangular */
    double m_width2;
    /** length of the component */
    double m_length;
    /** height of the component */
    double m_height;
  };

  /** Struct holding all parameters for a completeVXD Sensor */
  class VXDGeoSensor: public VXDGeoComponent {
  public:
    /** Constructor */
    VXDGeoSensor(const std::string& material = "", const std::string& color = "",
                 double width = 0, double width2 = 0, double length = 0, double height = 0, bool slanted = false):
      VXDGeoComponent(material, color, width, width2, length, height), m_info(0), m_slanted(slanted) {}
    /** set the active area
     * @param area component description of the active area
     * @param placement placement description of the active area
     */
    void setActive(const VXDGeoComponent& area, const VXDGeoPlacement& placement)
    {
      m_activeArea = area; m_activePlacement = placement;
    }
    /** get the component description for the active area */
    VXDGeoComponent& getActiveArea() { return m_activeArea; }
    /** get the placement description for the active area */
    const VXDGeoPlacement& getActivePlacement() { return m_activePlacement; }
    /** set the list of sub components to be placed */
    void setComponents(const std::vector<VXDGeoPlacement>& component) { m_components = component; }
    /** get the list of sub components */
    const std::vector<VXDGeoPlacement>& getComponents() const { return m_components; }
    /** set the pointer to the SensorInfo class */
    void setSensorInfo(VXD::SensorInfoBase* info) { m_info = info; }
    /** get the pointer to the SensorInfo class */
    const VXD::SensorInfoBase* getSensorInfo() const { return m_info; }
    /** return wether or not the sensor is slanted (usually only the first sensor in layers 4-6) */
    bool getSlanted() const { return m_slanted; }
  private:
    /** Paramerers of the active area */
    VXDGeoComponent m_activeArea;
    /** Position of the active area */
    VXDGeoPlacement m_activePlacement;
    /** Sensor Information instance */
    VXD::SensorInfoBase* m_info;
    /** Indication wether this is a slanted sensor */
    bool m_slanted;
    /** List of all components to be placed */
    std::vector<VXDGeoPlacement> m_components;
  };

  /** Struct holding the information where a sensor should be placed inside the ladder */
  class VXDGeoSensorPlacement {
  public:
    /** Constructor */
    VXDGeoSensorPlacement(int sensorID = 0, const std::string& sensorTypeID = "", double z = 0,
                          bool flipU = false, bool flipV = false, bool flipW = false):
      m_sensorID(sensorID), m_sensorTypeID(sensorTypeID), m_z(z), m_flipU(flipU), m_flipV(flipV), m_flipW(flipW) {}
    /** get the sensor id inside the ladder */
    int getSensorID() const { return m_sensorID; }
    /** get the sensor type id, internal id only needed for construction */
    const std::string& getSensorTypeID() const { return m_sensorTypeID; }
    /** get the z position of the sensor */
    double getZ() const { return m_z; }
    /** check whether or not the sensor should be flipped around the U coordinate */
    bool getFlipU() const { return m_flipU; }
    /** check whether or not the sensor should be flipped around the V coordinate */
    bool getFlipV() const { return m_flipV; }
    /** check whether or not the sensor should be flipped around the W coordinate */
    bool getFlipW() const { return m_flipW; }
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
  };

  /** Struct containing all parameters of one ladder */
  class VXDGeoLadder {
  public:
    /** Constructor */
    VXDGeoLadder(int layerID = 0, double shift = 0, double radius = 0,
                 double slantedAngle = 0, double slantedRadius = 0,
                 double glueSize = -1, const std::string& glueMaterial = ""):
      m_layerID(layerID), m_shift(shift), m_radius(radius),
      m_slantedAngle(slantedAngle), m_slantedRadius(slantedRadius),
      m_glueSize(glueSize), m_glueMaterial(glueMaterial) {}
    /** get the layer id */
    int getLayerID() const { return m_layerID; }
    /** get the shift along the u coordinate for all sensors in the ladder */
    double getShift() const { return m_shift; }
    /** get the radius of all sensors except slanted ones */
    double getRadius() const { return m_radius; }
    /** get the slant angle for slanted sensors */
    double getSlantedAngle() const { return m_slantedAngle; }
    /** get the radius for slanted sensors */
    double getSlantedRadius() const { return m_slantedRadius; }
    /** get the additional glue size, e.g. the amount the glue is produding on all sides of the joint */
    double getGlueSize() const { return m_glueSize; }
    /** get the glue material */
    const std::string& getGlueMaterial() const { return m_glueMaterial; }
    /** get list of sensors */
    const std::vector<VXDGeoSensorPlacement>& getSensors() const { return m_sensors; }
    /** add a sensor to the list of sensors in the ladder */
    void addSensor(const VXDGeoSensorPlacement& sensor) { m_sensors.push_back(sensor); }
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
    std::vector<VXDGeoSensorPlacement> m_sensors;
  };

} // namespace Belle2

#endif /* GEOVXDCOMPONENTS_H */
