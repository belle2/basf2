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
  * The Class for VXD Radiation Sensor Position parameters
  */
  class VXDGeoRadiationSensorsPositionPar: public TObject {
  public:
    /** Constructor */
    VXDGeoRadiationSensorsPositionPar(double z = 0, double radius = 0, double theta = 0) :
      m_z(z), m_radius(radius), m_theta(theta) {}
    //! Destructor
    ~VXDGeoRadiationSensorsPositionPar() {}
    /** get the common z position of the sensor centers  */
    double getZ() const { return m_z; }
    /** set the common z position of the sensor centers  */
    void  setZ(double z)  { m_z = z; }
    /** get the common radial position of the sensor centers  */
    double getRadius() const { return m_radius; }
    /** set the common radial position of the sensor centers  */
    void setRadius(double radius) { m_radius = radius; }
    /** get the common angle to the z axis, 0 is parallel to z in deg */
    double getTheta() const { return m_theta; }
    /** set the common angle to the z axis, 0 is parallel to z in deg */
    void setTheta(double theta)  { m_theta = theta; }
    /** get the vector storing ids of individual sensors */
    const std::map<int, double>& getSensors() const { return  m_sensors; }
    /** add sensor with individual id */
    void addSensor(int id, double phi)  { m_sensors[id] = phi; }

  private:
    //! Common z position of the sensor centers in mm
    double m_z;
    //! Common radial position of the sensor centers in mm
    double m_radius;
    //! Common angle to the z axis, 0 is parallel to z in deg
    double m_theta;
    //! Individual radiations sensors
    std::map<int, double> m_sensors;


    ClassDef(VXDGeoRadiationSensorsPositionPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };

  /**
  * The Class for VXD Radiation Sensor parameters
  */
  class VXDGeoRadiationSensorsPar: public TObject {
  public:
    /** Constructor */
    VXDGeoRadiationSensorsPar(const std::string& subdetector = "", bool insideEnvelope = false,
                              double width = 0, double length = 0, bool height = 0, const std::string& material = ""):
      m_subdetector(subdetector), m_insideEnvelope(insideEnvelope), m_width(width), m_length(length), m_height(height),
      m_material(material) {}
    //! Destructor
    ~VXDGeoRadiationSensorsPar() {}
    /** get the name of the subdetector */
    const std::string& getSubDetector() const { return  m_subdetector; }
    /** set the name of the subdetector */
    void setSubDetector(const std::string& subdetector) { m_subdetector = subdetector; }
    /** return whether radiation sensor are inside envelope volume */
    bool getInsideEnvelope() const { return   m_insideEnvelope; }
    /** set whether radiation sensor are inside envelope volume */
    void setInsideEnvelope(bool insideEnvelope) { m_insideEnvelope = insideEnvelope; }
    /** get the width */
    double getWidth() const { return m_width; }
    /** set the width */
    void setWidth(double width) {m_width = width; }
    /** get the length */
    double getLength() const { return m_length; }
    /** set the length */
    void setLength(double length) { m_length = length; }
    /** get the height */
    double getHeight() const { return m_height; }
    /** set the height */
    void setHeight(double height) { m_height = height; }
    /** get the name of the material */
    const std::string& getMaterial() const { return m_material; }
    /** set the name of the material */
    void setMaterial(const std::string& material) { m_material = material; }
    /** get the positions of radiation sensors */
    const std::vector<VXDGeoRadiationSensorsPositionPar>& getPositions() const { return m_positions; }
    /** add radiation sensor position */
    void addPosition(const VXDGeoRadiationSensorsPositionPar& position)  {  m_positions.push_back(position); }

  private:
    //! one of "PXD" or "SVD"
    std::string m_subdetector;
    //! For PXD they need to be outside the envelope volume, for SVD they are inside the detector.
    bool m_insideEnvelope;
    //! Dimension of the sensor in mm
    double m_width;
    //! Dimension of the sensor in mm
    double m_length;
    //! Dimension of the sensor in um
    double m_height;
    //! Material name
    std::string m_material;
    //! Position of radiation sensors
    std::vector<VXDGeoRadiationSensorsPositionPar> m_positions;

    ClassDef(VXDGeoRadiationSensorsPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2

