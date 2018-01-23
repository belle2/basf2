/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2012  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Heck, Martin Ritter                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <string>
#include <map>

namespace Belle2 {

  /**
   * The Unit class.
   *
   * This class provides a set of units for the framework. Use these to
   * specify the unit of your value. In addition the class offers methods
   * to convert floating point numbers from a specified unit to the standard
   * unit of the framework.
   *
   * \code
     //multiply with a Unit to convert to standard units, e.g.
     double length = 10.0 * Unit::mm;
     // length now equals 1.0 (stand for length: cm)

     //divide with a Unit to convert from standard units to the given one:
     double length_mm  = length / Unit::mm;
     \endcode
   *
   * This class cannot be instantiated, use the static members directly.
   * The implementation can be found in UnitConst.cc
   */
  class Unit {

  public:

    /**
     * Standard units with the value = 1.
     */
    static const double cm;           /**< Standard of [length] */
    static const double ns;           /**< Standard of [time] */
    static const double GHz;          /**< Standard of [frequency] */
    static const double rad;          /**< Standard of [angle] */
    static const double GeV;          /**< Standard of [energy, momentum, mass] */
    static const double K;            /**< Standard of [temperature] */
    static const double e;            /**< Standard of [electric charge] */

    /**
     * Practical units with the value set at 1.
     * These units are NOT consistent with the other standard units.
     * Please use them carefully.
     */
    static const double g_cm3;        /**< Standard of [density] */


    /**
     * Derived units.
     */

    // length units
    static const double km;           /**< [kilometers] */
    static const double m;            /**< [meters] */
    static const double mm;           /**< [millimeters] */
    static const double um;           /**< [micrometers] */
    static const double nm;           /**< [nanometers] */
    static const double pm;           /**< [picometers] */
    static const double fm;           /**< [femtometers] */

    // area units
    static const double m2;           /**< [square meters] */
    static const double cm2;          /**< [square centimeters] */
    static const double mm2;          /**< [square millimeters] */

    static const double b;            /**< [barn] */
    static const double mb;           /**< [millibarn] */
    static const double ub;           /**< [microbarn] */
    static const double nb;           /**< [nanobarn] */
    static const double pb;           /**< [picobarn] */
    static const double fb;           /**< [femtobarn] */
    static const double ab;           /**< [attobarn] */

    // volume units
    static const double m3;           /**< [cubic meters] */
    static const double cm3;          /**< [cubic centimeters] */
    static const double mm3;          /**< [cubic millimeters] */

    // time units
    static const double s;            /**< [second] */
    static const double ms;           /**< [millisecond] */
    static const double us;           /**< [microsecond] */
    static const double ps;           /**< [picosecond] */
    static const double fs;           /**< [femtosecond] */

    // frequency units
    static const double Hz;           /**< [Hertz] */
    static const double kHz;          /**< [Kilohertz] */
    static const double MHz;          /**< [Megahertz] */
    static const double mHz;          /**< [Millihertz] */

    // angle units
    static const double mrad;         /**< [millirad] */
    static const double deg;          /**< degree to radians */

    // energy units
    static const double eV;           /**< [electronvolt] */
    static const double keV;          /**< [kiloelectronvolt] */
    static const double MeV;          /**< [megaelectronvolt] */
    static const double TeV;          /**< [teraelectronvolt] */
    static const double J;            /**< [joule] */
    static const double V;            /**< [voltage] */

    // magnetic field units
    static const double T;            /**< [tesla] */
    static const double mT;           /**< [millitesla] */
    static const double Gauss;        /**< [gauss] */
    static const double kGauss;       /**< [kilogauss] */

    // charge units
    static const double C;            /**< [coulomb] */
    static const double fC;           /**< [femtocoulomb] */

    // density units
    static const double mg_cm3;       /**< [mg/cm^3] */
    static const double kg_cm3;       /**< [kg/cm^3] */
    static const double g_mm3;        /**< [g/mm^3] */
    static const double mg_mm3;       /**< [mg/mm^3] */
    static const double kg_mm3;       /**< [kg/mm^3] */


    /**
     * Converts a floating point value to the standard framework unit.
     *
     * If the given unit is not found, the value is returned unchanged (and an error printed).
     *
     * @param value The value which should be converted.
     * @param unitString The string of the unit of the specified value.
     * @return The value converted to the standard unit.
     */
    static double convertValue(double value, const std::string& unitString);

    /**
     * Converts a floating point value from the standard framework unit to the given unit.
     *
     * If the given unit is not found, the value is returned unchanged (and an error printed).
     *
     * @param value The value which should be converted.
     * @param unitString The string of the unit of the specified value.
     * @return The value converted to the given unit.
     */
    static double convertValueToUnit(double value, const std::string& unitString);

  protected:
    /** registers the name and value of a conversion in s_conversionFactors. */
    static double registerConversion(const std::string& name, double value);
    /** Map to be able to convert between units using string representations of the unit name */
    static std::map<std::string, double> s_conversionFactors;

  private:
    /**
     * @{
     * no Unit instances allowed.
     */
    Unit();
    Unit(const Unit&);
    Unit& operator=(const Unit&);
    ~Unit() {};
    /** @} */
  };

}
