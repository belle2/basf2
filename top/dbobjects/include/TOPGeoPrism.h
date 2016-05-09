/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <top/dbobjects/TOPGeoBarSegment.h>

namespace Belle2 {

  /**
   *
   */
  class TOPGeoPrism: public TOPGeoBarSegment {
  public:

    /**
     * Default constructor
     */
    TOPGeoPrism()
    {}

    /**
     * Useful constructor
     * @param width prism width
     * @param thickness thickness at bar side
     * @param length prism length
     * @param exitThickness thickness at PMT side
     * @param flatLenght length of the flat part at the bottom
     * @param material prism material name
     */
    TOPGeoPrism(double width, double thickness, double length,
                double exitThickness, double flatLength,
                const std::string& material):
      TOPGeoBarSegment(width, thickness, length, material),
      m_exitThickness(exitThickness), m_flatLength(flatLength)
    {}

    /**
     * Returns prism thickness at PMT side
     * @return thickness at PMT side
     */
    double getExitThickness() const {return m_exitThickness / s_unit;}

    /**
     * Returns the length of a flat surface at prism bottom
     * @return bottom flat surface length
     */
    double getFlatLength() const {return m_flatLength / s_unit;}

    /**
     * Returns the length of a flat surface at prism bottom including filter
     * @return bottom flat surface full length
     */
    double getFullFlatLength() const {return (m_flatLength + m_glueThickness) / s_unit;}

    /**
     * Returns wavelength filter thickness (filter on -z side)
     * @return thickness
     */
    double getFilterThickness() const {return m_glueThickness / s_unit;}

    /**
     * Returns wavelength filter material name (filter on -z side)
     * @return material name
     */
    const std::string& getFilterMaterial() const {return m_glueMaterial;}

    /**
     * Returns glue thickness (no glue on -z side, instead there is a wavelength filter)
     * @return thickness
     */
    double getGlueThickness() const {return 0;}

    /**
     * Check for consistency of data members
     * @return true if values consistent (valid)
     */
    bool isConsistent() const;

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "Prism geometry parameters") const;


  private:

    float m_exitThickness = 0; /**< thickness at PMT side */
    float m_flatLength = 0; /**< length of the flat part at the bottom */

    ClassDef(TOPGeoPrism, 1); /**< ClassDef */

  };

} // end namespace Belle2
