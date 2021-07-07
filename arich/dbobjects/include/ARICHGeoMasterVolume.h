/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <arich/dbobjects/ARICHGeoBase.h>
#include <string>
#include <TVector3.h>
#include <TRotation.h>

namespace Belle2 {


  /**
   * Geometry parameters of ARICH Master volume (envelope)
   */


  class ARICHGeoMasterVolume: public ARICHGeoBase {

  public:

    /**
     * Default constructor
     */
    ARICHGeoMasterVolume()
    {}

    /**
     * Destructor
     */
    ~ARICHGeoMasterVolume()
    {
      /*if(m_rotation) delete m_rotation;
      if(m_rotationInverse) delete m_rotationInverse;
      if(m_translation) delete m_translation;
      */
    }

    /**
     * Check of geometry parameters consistency
     * @return true if consistent
     */
    bool isConsistent() const override;

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "ARICH Master Volume geometry parameters") const override;

    /**
     * Sets positioning of ARICH master volume in global Belle II coordinate system
     * @param x x position
     * @param y y position
     * @param z z position
     * @param rx rotation around x-axis
     * @param ry rotation around y-axis
     * @param rz rotation around z-axis
     */
    void setPlacement(double x, double y, double z, double rx, double ry, double rz);

    /**
     * Sets parameters of ARICH master volume
     * @param innerR inner radius of ARICH tube
     * @param outerR outer radius of ARICH tube
     * @param length length radius of ARICH tube
     * @param material material
     */
    void setVolume(double innerR, double outerR, double length, const std::string& material) {m_innerR = innerR; m_outerR = outerR; m_length = length; m_material = material;};

    /**
     * Get position of ARICH master volume center point in global Belle II coordinates
     * @return center point of ARICH volume
     */
    TVector3 getPosition() const {return TVector3(m_x / s_unit, m_y / s_unit, m_z / s_unit);}

    /**
     * Get rotation matrix of ARICH master volume in global Belle II coordinates
     * @return rotation matrix of ARICH master volume
     */
    TRotation getRotation() const
    {
      TRotation rot;
      rot.RotateX(m_rx).RotateY(m_ry).RotateZ(m_rz);
      return rot;
    }

    /**
     * Get angle of rotation around X axis
     * @return rotation angle arounx X axis
     */
    double getRotationX() const {return m_rx;}

    /**
     * Get angle of rotation around Y axis
     * @return rotation angle arounx Y axis
     */
    double getRotationY() const {return m_ry;}

    /**
     * Get angle of rotation around Z axis
     * @return rotation angle arounx Z axis
     */
    double getRotationZ() const {return m_rz;}

    /**
     * Get ARICH master volume inner radius
     * @return master volume inner radius
     */
    double getInnerRadius() const {return m_innerR / s_unit;};

    /**
    * Get ARICH master volume outer radius
    * @return master volume outer radius
    */
    double getOuterRadius() const {return m_outerR / s_unit;};

    /**
     * Get ARICH master volume length
     * @return master volume length
     */
    double getLength() const {return m_length / s_unit;};

    /**
     * Get material of ARICH master volume
     * @return material of master volume
     */
    const std::string& getMaterial() const {return m_material;}

    TVector3 pointToGlobal(const TVector3& point) const;
    TVector3 momentumToGlobal(const TVector3& momentum) const;
    TVector3 pointToLocal(const TVector3& point) const;
    TVector3 momentumToLocal(const TVector3& momentum) const;


  private:
    // position in global BelleII frame
    double m_x = 0;  /**< x position */
    double m_y = 0;  /**< y position */
    double m_z = 0;  /**< z position */

    // rotations in global BelleII frame
    double m_rx = 0; /**< rotation around x-axis */
    double m_ry = 0; /**< rotation around y-axis */
    double m_rz = 0; /**< rotation around z-axis */

    double m_innerR = 0; /**< tube inner radius */
    double m_outerR = 0; /**< tube outer radius */
    double m_length = 0; /**< tube length */

    std::string m_material;

    mutable  TRotation* m_rotation = 0 ;
    mutable TRotation* m_rotationInverse = 0;
    mutable TVector3*  m_translation = 0;

    ClassDefOverride(ARICHGeoMasterVolume, 1); /**< ClassDef */

  };

} // end namespace Belle2
