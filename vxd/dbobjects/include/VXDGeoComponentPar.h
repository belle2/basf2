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
#include <vector>

#include <algorithm>


class G4LogicalVolume;

namespace Belle2 {

  class GearDir;


  /**
  * The Class for VXD geometry component
  */
  class VXDGeoComponentPar: public TObject {

  public:
    /** Constructor */
    VXDGeoComponentPar(const std::string& material = "", const std::string& color = "",
                       double width = 0, double width2 = 0, double length = 0, double height = 0, double angle = 0):
      m_volume(0), m_material(material), m_color(color), m_width(width), m_width2(width2), m_length(length),
      m_height(height), m_angle(angle)
    {
      //Consistency check: Otherwise dimensions are silently changed in createTrapezoidal() ...
      if (m_angle > 0) {
        const double tana = tan(m_angle);
        m_height = std::min(tana * m_length, std::min(tana * m_width, m_height));
      }
    }

    //! Destructor
    ~VXDGeoComponentPar() {}
    /** get the pointer to the logical volume, NULL if not yet created */
    G4LogicalVolume* getVolume() const { return m_volume; }
    /** set the pointer to the logical volume */
    void setVolume(G4LogicalVolume* volume) { m_volume = volume; }
    /** get the name of the Material for the component */
    const std::string& getMaterial() const { return m_material; }
    /** set the name of the Material for the component */
    void setMaterial(const std::string& material) { m_material = material; }
    /** get the name of the color for the component */
    const std::string& getColor() const { return m_color; }
    /** set the name of the color for the component */
    void setColor(const std::string& color) {m_color = color; }
    /** get the width of the component */
    double getWidth() const { return m_width; }
    /** set the width of the component. This will automatically set width2 to zero */
    void setWidth(double width) { m_width = width; m_width2 = 0; }
    /** get the forward width of the component, 0 for rectangular */
    double getWidth2() const { return m_width2; }
    /** set the width2 of the component */
    void setWidth2(double width2) {m_width2 = width2; }
    /** get the length of the component */
    double getLength() const { return m_length; }
    /** set the length of the component */
    void setLength(double length) { m_length = length; }
    /** get the height of the component */
    double getHeight() const { return m_height; }
    /** get the height of the component */
    double& getHeight() { return m_height; }
    /** set the height of the component */
    void setHeight(double height) { m_height = height; }
    /** get the angle of the component */
    double getAngle() const { return m_angle; }
    /** set the angle of the component */
    void setAngle(double angle) { m_angle = angle; }

  private:

    /** Pointer to the Logical volume if already created  */
    G4LogicalVolume* m_volume {0}; //! transient member, owned by Geant4
    /** Name of the material of the component */
    std::string m_material {""};
    /** Name of the color of the component */
    std::string m_color {""} ;
    /** width of the component */
    double m_width {0.};
    /** forward width of the component, 0 for recangular */
    double m_width2 {0.};
    /** length of the component */
    double m_length {0.};
    /** height of the component */
    double m_height {0.};
    /** angle of the component */
    double m_angle {0.};

    ClassDef(VXDGeoComponentPar, 5);  /**< ClassDef, must be the last term before the closing {}*/

  };
} // end of namespace Belle2

