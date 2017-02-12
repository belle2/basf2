/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <geometry/dbobjects/GeoOpticalSurface.h>
#include <string>
#include <framework/gearbox/Unit.h>


namespace Belle2 {

  /**
   * Base class for geometry parameters.
   */
  class ARICHGeoBase: public TObject {
  public:

    /**
     * Default constructor
     */
    ARICHGeoBase() {};

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
     * Print volume positioning parameters
     * @param x x position
     * @param y y position
     * @param z z position
     * @param rx rotation around x-axis
     * @param ry rotation around y-axis
     * @param rz rotation around z-axis
     */
    virtual void printPlacement(double x, double y, double z, double rx, double ry, double rz) const;

  protected:

    static double s_unit; /**< conversion unit for length */
    static std::string s_unitName; /**< conversion unit name */

    ClassDef(ARICHGeoBase, 1); /**< ClassDef */

  };

} // end namespace Belle2
