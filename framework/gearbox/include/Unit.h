/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2012  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Heck, Martin Ritter                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef UNIT_H
#define UNIT_H

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
   * This class cannot be instantiated, use the static members directly.
   */
  class Unit {

  public:

    // standard units
    static const double cm;           /**< Standard of [length] */
    static const double ns;           /**< Standard of [time] */
    static const double rad;          /**< Standard of [angle] */
    static const double GeV;          /**< Standard of [energy, momentum, mass] */
    static const double K;            /**< Standard of [temperature] */
    static const double T;            /**< Standard of [magnetic field], WARNING: This is inconsistent with the other standard units. Use TinStdUnits for conversion. */
    static const double e;            /**< Standard of [electric charge] */
    static const double g_cm3;        /**< Standard of [density] */

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

    // charge units
    static const double C;            /**< [coulomb] */
    static const double fC;           /**< [femtocoulomb] */

    // density units
    static const double mg_cm3;       /**< [mg/cm^3] */
    static const double kg_cm3;       /**< [kg/cm^3] */
    static const double g_mm3;        /**< [g/mm^3] */
    static const double mg_mm3;       /**< [mg/mm^3] */
    static const double kg_mm3;       /**< [kg/mm^3] */

    // magnetic field units
    static const double TinStdUnits;  /**< [Vs/m^2] */

    //Various constants
    static const double speedOfLight; /**< [cm/ns] */
    static const double kBoltzmann;   /**< Boltzmann constant in GeV/K. */
    static const double ehEnergy;     /**< Energy needed to create an electron-hole pair in Si at std. T. */
    static const double electronMass; /**< Electron mass in MeV. */
    static const double fineStrConst; /**< The fine structure constant. */
    static const double permSi;       /**< Permittivity of Silicon */
    static const double uTherm;       /**< Thermal Voltage at room temperature */
    static const double eMobilitySi;  /**< Electron mobility in intrinsic Silicon at room temperature */

    /** Trackable Particles ID.
     *
     *  Pion is the first one, because usually pions are the most common particles and therefore often the standard.
     */
    enum EChargedStable {
      c_Pion     = 0, /** Assume a Pion.      */
      c_Kaon     = 1, /** Assume a Kaon.      */
      c_Proton   = 2, /** Assume a Proton.    */
      c_Electron = 3, /** Assume an Electron. */
      c_Muon     = 4  /** Assume a Muon.      */
    };

    /** Very often the mass of charged stable Particles is needed. */
    static float chargedStableMass(const EChargedStable chargedStable = c_Pion);



    //SuperKEKB and Belle II constants
    static const double crossingAngleLER; /**< The crossing angle of the LER. */
    static const double crossingAngleHER; /**< The crossing angle of the LER. */


    /**
     * Converts a floating point value to the standard framework unit.
     *
     * @param value The value which should be converted.
     * @param unitString The string of the unit of the specified value.
     * @return The value converted to the specified unit.
     */
    static double convertValue(double value, const std::string& unitString);

  protected:
    /** registers the name and value of a conversion in m_conversionFactors. */
    static double registerConversion(const std::string& name, double value);
    /** Map to be able to convert between units using string representations of the unit name */
    static std::map<std::string, double> m_conversionFactors;

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

#endif /* UNIT_H */
