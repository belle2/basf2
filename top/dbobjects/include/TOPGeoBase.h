/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <geometry/dbobjects/GeoOpticalSurface.h>
#include <string>


namespace Belle2 {

  /**
   * Base class for geometry parameters.
   */
  class TOPGeoBase: public TObject {
  public:

    /**
     * Default constructor
     */
    TOPGeoBase()
    {}

    /**
     * Constructor with name
     * @param name object name
     */
    explicit TOPGeoBase(const std::string& name): m_name(name)
    {}

    /**
     * Sets object name
     * @param name object name
     */
    void setName(const std::string& name) {m_name = name;}

    /**
     * Returns object name
     * @return object name
     */
    const std::string& getName() const {return m_name;}

    /**
     * Check for consistency of data members
     * @return true if values consistent (valid)
     */
    virtual bool isConsistent() const {return false;}

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    virtual void print(const std::string& title) const;

    /**
     * Print the content of optical surface
     * @param surface optical surface parameters
     */
    virtual void printSurface(const GeoOpticalSurface& surface) const;

    /**
     * Returns reflectivity of optical surface at given photon energy
     * @param surface optical surface parameters
     * @param energy photon energy in [eV]
     */
    double getReflectivity(const GeoOpticalSurface& surface, double energy) const;

  protected:

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void printUnderlined(const std::string& title) const;

    static double s_unit; /**< conversion unit for length */
    static std::string s_unitName; /**< conversion unit name */

    std::string m_name; /**< geometry object name */

    ClassDef(TOPGeoBase, 1); /**< ClassDef */

  };

} // end namespace Belle2
