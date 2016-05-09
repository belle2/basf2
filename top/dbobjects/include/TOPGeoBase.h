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


  protected:

    static double s_unit; /**< conversion unit for length */
    static std::string s_unitName; /**< conversion unit name */

    ClassDef(TOPGeoBase, 1); /**< ClassDef */

  };

} // end namespace Belle2
