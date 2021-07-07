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
   * Geometry parameters of cold plate (simplified)
   */
  class TOPGeoColdPlate: public TOPGeoBase {
  public:

    /**
     * Default constructor
     */
    TOPGeoColdPlate()
    {}

    /**
     * Full constructor
     * @param baseThickness base plate thickness
     * @param baseMaterial base plate material name
     * @param coolThickness cooling plate thickness
     * @param coolWidth cooling plate width
     * @param coolMaterial cooling plate material name
     * @param name volume name in Geant
     */
    TOPGeoColdPlate(double baseThickness,
                    const std::string& baseMaterial,
                    double coolThickness,
                    double coolWidth,
                    const std::string& coolMaterial,
                    const std::string& name = "TOPColdPlate"): TOPGeoBase(name),
      m_baseThickness(baseThickness), m_baseMaterial(baseMaterial),
      m_coolThickness(coolThickness), m_coolWidth(coolWidth),
      m_coolMaterial(coolMaterial)
    {}

    /**
     * Returns base plate thickness
     * @return thickness
     */
    double getBaseThickness() const {return m_baseThickness / s_unit;}

    /**
     * Returns base plate material name
     * @return material name
     */
    const std::string& getBaseMaterial() const {return m_baseMaterial;}

    /**
     * Returns cooling plate thickness
     * @return thickness
     */
    double getCoolThickness() const {return m_coolThickness / s_unit;}

    /**
     * Returns cooling plate width
     * @return width
     */
    double getCoolWidth() const {return m_coolWidth / s_unit;}

    /**
     * Returns cooling plate material name
     * @return material name
     */
    const std::string& getCoolMaterial() const {return m_coolMaterial;}

    /**
     * Check for consistency of data members
     * @return true if values consistent (valid)
     */
    bool isConsistent() const override;

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    virtual void print(const std::string& title = "QBB cold plate geometry parameters") const override;



  private:

    float m_baseThickness = 0;  /**< base plate thickness */
    std::string m_baseMaterial; /**< base plate material name */
    float m_coolThickness = 0;  /**< cooling plate thickness */
    float m_coolWidth = 0;      /**< cooling plate width */
    std::string m_coolMaterial; /**< cooling plate material name */

    ClassDefOverride(TOPGeoColdPlate, 1); /**< ClassDef */

  };

} // namespace Belle2
