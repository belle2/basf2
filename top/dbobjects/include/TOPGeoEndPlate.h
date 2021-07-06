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
   * Geometry parameters of forward end plate (simplified)
   */
  class TOPGeoEndPlate: public TOPGeoBase {
  public:

    /**
     * Default constructor
     */
    TOPGeoEndPlate()
    {}

    /**
     * Full constructor
     * @param thickness thickness
     * @param height height
     * @param material material name
     * @param name volume name in Geant
     */
    TOPGeoEndPlate(double thickness, double height,
                   const std::string& material,
                   const std::string& name = "TOPEndPlate"): TOPGeoBase(name),
      m_thickness(thickness), m_height(height), m_material(material)
    {}

    /**
     * Returns thickness
     * @return theckness
     */
    double getThickness() const {return m_thickness / s_unit;}

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
    virtual void print(const std::string& title = "QBB forward end plate geometry parameters") const override;

  private:

    float m_thickness = 0; /**< thickness */
    float m_height = 0; /**< height */
    std::string m_material; /**< material name */

    ClassDefOverride(TOPGeoEndPlate, 1); /**< ClassDef */

  };

} // namespace Belle2
