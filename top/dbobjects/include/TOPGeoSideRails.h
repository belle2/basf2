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
   * Geometry parameters of side rails (simplified)
   */
  class TOPGeoSideRails: public TOPGeoBase {
  public:

    /**
     * Default constructor
     */
    TOPGeoSideRails()
    {}

    /**
     * Full constructor
     * @param thickness thickness
     * @param reducedThickness thickness at prism enclosure
     * @param height height
     * @param material material name
     * @param name volume name in Geant
     */
    TOPGeoSideRails(double thickness, double reducedThickness, double height,
                    const std::string& material,
                    const std::string& name = "TOPSideRail"): TOPGeoBase(name),
      m_thickness(thickness), m_reducedThickness(reducedThickness),
      m_height(height), m_material(material)
    {}

    /**
     * Returns thickness
     * @return theckness
     */
    double getThickness() const {return m_thickness / s_unit;}

    /**
     * Returns thickness at prism enclosure
     * @return theckness
     */
    double getReducedThickness() const {return m_reducedThickness / s_unit;}

    /**
     * Returns height
     * @return heihgt
     */
    double getHeight() const {return m_height / s_unit;}

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
    virtual void print(const std::string& title = "QBB side rails geometry parameters") const override;

  private:

    float m_thickness = 0; /**< thickness */
    float m_reducedThickness = 0; /**< thickness at prism enclosure */
    float m_height = 0; /**< height */
    std::string m_material; /**< material name */

    ClassDefOverride(TOPGeoSideRails, 1); /**< ClassDef */

  };

} // namespace Belle2
