/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <top/dbobjects/TOPGeoBase.h>
#include <string>

namespace Belle2 {

  /**
   * Geometry parameters of prism enclosure (simplified)
   */
  class TOPGeoPrismEnclosure: public TOPGeoBase {
  public:

    /**
     * Default constructor
     */
    TOPGeoPrismEnclosure()
    {}

    /**
     * Full constructor
     * @param length length
     * @param height height up to the side rails
     * @param angle angle of side wall
     * @param bottomThickness bottom thickness
     * @param sideThickness side wall thickness
     * @param backThickness back wall thickness
     * @param frontThickness front wall thickness
     * @param extensionThickness extension plate thickness
     * @param material material name
     * @param name volume name in Geant
     */
    TOPGeoPrismEnclosure(double length,
                         double height,
                         double angle,
                         double bottomThickness,
                         double sideThickness,
                         double backThickness,
                         double frontThickness,
                         double extensionThickness,
                         const std::string& material,
                         const std::string& name = "TOPPrismEnclosure"): TOPGeoBase(name),
      m_length(length), m_height(height), m_angle(angle),
      m_bottomThickness(bottomThickness), m_sideThickness(sideThickness),
      m_backThickness(backThickness), m_frontThickness(frontThickness),
      m_extensionThickness(extensionThickness),
      m_material(material)
    {}

    /**
     * Returns full length
     * @return length
     */
    double getLength() const {return m_length / s_unit;}

    /**
     * Returns length w/o back and front plates
     * @return length
     */
    double getBodyLength() const
    {
      return getLength() - getBackThickness() - getFrontThickness();
    }

    /**
     * Returns height up to the side rails
     * @return height
     */
    double getHeight() const {return m_height / s_unit;}

    /**
     * Returns angle of side wall
     * @return angle
     */
    double getAngle() const {return m_angle;}

    /**
     * Returns bottom thickness
     * @return thickness
     */
    double getBottomThickness() const {return m_bottomThickness / s_unit;}

    /**
     * Returns side wall thickness
     * @return thickness
     */
    double getSideThickness() const {return m_sideThickness / s_unit;}

    /**
     * Returns back wall thickness
     * @return thickness
     */
    double getBackThickness() const {return m_backThickness / s_unit;}

    /**
     * Returns front wall thickness
     * @return thickness
     */
    double getFrontThickness() const {return m_frontThickness / s_unit;}

    /**
     * Returns extension plate thickness
     * @return thickness
     */
    double getExtensionThickness() const {return m_extensionThickness / s_unit;}

    /**
     * Returns material name
     * @return material name
     */
    const std::string& getMaterial() const {return m_material;}

    /**
     * Check for consistency of data members
     * @return true if values consistent (valid)
     */
    bool isConsistent() const override;

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    virtual void print(const std::string& title = "QBB prism enclosure geometry parameters") const override;

  private:

    float m_length = 0; /**< length */
    float m_height = 0; /**< height up to the side rails */
    float m_angle = 0;  /**< angle of side wall */
    float m_bottomThickness = 0; /**< bottom thickness */
    float m_sideThickness = 0;   /**< side wall thickness */
    float m_backThickness = 0;   /**< back wall thickness */
    float m_frontThickness = 0;  /**< front wall thickness */
    float m_extensionThickness = 0;  /**< extension plate thickness */
    std::string m_material; /**< material name */

    ClassDefOverride(TOPGeoPrismEnclosure, 1); /**< ClassDef */

  };

} // namespace Belle2
