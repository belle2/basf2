/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef UNIT_H_
#define UNIT_H_

#include <TMath.h>

#include <string>
#include <map>
#include <cmath>

namespace Belle2 {

  /**
   * The Unit class.
   *
   * This class provides a set of units for the framework. Use these to
   * specify the unit of your value. In addition the class offers methods
   * to convert floating point numbers from a specified unit to the standard
   * unit of the framework.
   *
   * This class can either be used a singleton or by using its static methods.
   */
  class Unit {

  public:

    // standard units
    static double cm;   /**< Standard of [length] */
    static double ns;   /**< Standard of [time] */
    static double rad;  /**< Standard of [angle] */
    static double GeV;  /**< Standard of [energy, momentum, mass] */
    static double K;    /**< Standard of [temperature] */
    static double T;    /**< Standard of [magnetic field] */
    static double e;    /**< Standard of [electric charge] */
    static double gcm3; /**< Standard of [density] */

    // length units
    static double km; /**< [kilometers] */
    static double m;  /**< [meters] */
    static double mm; /**< [millimeters] */
    static double um; /**< [micrometers] */
    static double nm; /**< [nanometers] */
    static double fm; /**< [femtometers] */

    // area units
    static double m2;  /**< [square meters] */
    static double cm2; /**< [square centimeters] */
    static double mm2; /**< [square millimeters] */

    static double b;  /**< [barn] */
    static double mb; /**< [millibarn] */
    static double ub; /**< [microbarn] */
    static double nb; /**< [nanobarn] */
    static double pb; /**< [picobarn] */
    static double fb; /**< [femtobarn] */
    static double ab; /**< [atobarn] */

    // volume units
    static double m3;  /**< [cubic meters] */
    static double cm3; /**< [cubic centimeters] */
    static double mm3; /**< [cubic millimeters] */

    // time units
    static double s;   /**< [second] */
    static double ms;  /**< [millisecond] */
    static double us;  /**< [microsecond] */
    static double ps;  /**< [picosecond] */
    static double fs;  /**< [femtosecond] */

    // angle units
    static double mrad; /**< [millirad] */
    static double deg;  /**< degree to radians */

    // energy units
    static double eV;  /**< [electronvolt] */
    static double keV; /**< [kiloelectronvolt] */
    static double MeV; /**< [megaelectronvolt] */
    static double TeV; /**< [megaelectronvolt] */
    static double J;   /**< [joule] */

    // charge units
    static double C;   /**< [coulomb] */
    static double fC;  /**< [femtocoulomb] */

    // density units
    static double mgcm3; /**< [mg/cm^3] */
    static double kgcm3; /**< [kg/cm^3] */
    static double gmm3;  /**< [g/mm^3] */
    static double mgmm3; /**< [mg/mm^3] */
    static double kgmm3; /**< [kg/mm^3] */

    //Various constants
    static double speed_of_light; /**< [cm/ns] */
    static double k_boltzmann;    /**< Boltzmann constant in GeV/K */
    static double energy_eh;      /**< Energy needed to create an electron-hole pair in Si at std. T */
    static double electron_mass;  /**< Electron mass in MeV */
    static double fine_str_const; /**< The fine structure constant */

    /** Definition of the supported units. */
    enum EUnitTypes { c_UnitLength, /**< length unit, default [cm]. */
                      c_UnitAngle,  /**< angle unit, default [rad]. */
                      c_UnitEnergy, /**< energy unit, default [GeV]. */
                      c_UnitDensity /**< density unit, default [g/cm3]. */
                    };

    /** Definition of the supported length units. */
    enum ELengthUnitTypes { c_UM,  /**< length unit [micrometer]. */
                            c_MM,  /**< length unit [mm]. */
                            c_CM,  /**< length unit [cm]. */
                            c_M,   /**< length unit [m]. */
                            c_KM   /**< length unit [km]. */
                          };

    /** Definition of the supported length units. */
    enum EAngleUnitTypes { c_Deg,  /**< angle unit [deg]. */
                           c_Rad,  /**< angle unit [rad]. */
                           c_MRad  /**< angle unit [mrad]. */
                         };

    /** Definition of the supported energy units. */
    enum EEnergyUnitTypes { c_eV,  /**< energy unit [eV]. */
                            c_keV, /**< energy unit [keV]. */
                            c_MeV, /**< energy unit [MeV]. */
                            c_GeV, /**< energy unit [GeV]. */
                            c_TeV, /**< energy unit [TeV]. */
                            c_Joule /**< energy unit [J].  */
                          };

    /** Definition of the supported density units. */
    enum EDensityUnitTypes { c_GCM,  /**< density unit [g/cm3]. */
                             c_MGCM, /**< energy unit [mg/cm3]. */
                             c_KGCM, /**< energy unit [kg/cm3]. */
                             c_GMM,  /**< energy unit [g/mm3]. */
                             c_MGMM, /**< energy unit [mg/mm3]. */
                             c_KGMM  /**< energy unit [kg/mm3]. */
                           };

    /**
     * Static method to get a reference to the UnitConverter instance.
     *
     * @return A reference to an instance of this class.
     */
    static Unit& Instance();


    /**
     * Converts a floating point value to the standard framework unit.
     *
     * @param value The value which should be converted.
     * @param unitType The unit type of the specified value.
     * @param unitString The string of the unit of the specified value.
     * @return The value converted to the specified unit.
     */
    static double convertValue(double value, EUnitTypes unitType, const std::string& unitString);

    /**
     * Converts a length value to the standard framework length unit [cm].
     *
     * @param value The value which should be converted.
     * @param unitString The string of the unit of the specified value.
     * @return The value converted to the specified unit.
     */
    static double convertLength(double value, const std::string& unitString);

    /**
     * Converts an angle value to the standard framework angle unit [rad].
     *
     * @param value The value which should be converted.
     * @param unitString The string of the unit of the specified value.
     * @return The value converted to the specified unit.
     */
    static double convertAngle(double value, const std::string& unitString);

    /**
     * Converts an energy value to the standard framework energy unit [GeV].
     *
     * @param value The value which should be converted.
     * @param unitString The string of the unit of the specified value.
     * @return The value converted to the specified unit.
     */
    static double convertEnergy(double value, const std::string& unitString);

    /**
     * Converts a density value to the standard framework energy unit [g/cm3].
     *
     * @param value The value which should be converted.
     * @param unitString The string of the unit of the specified value.
     * @return The value converted to the specified unit.
     */
    static double convertDensity(double value, const std::string& unitString);


  protected:

    /** Fills the map which links the string representing a length unit to the type if the unit. */
    void setLengthUnitMap();

    /** Fills the map which links the string representing an angle unit to the type if the unit. */
    void setAngleUnitMap();

    /** Fills the map which links the string representing an energy unit to the type if the unit. */
    void setEnergyUnitMap();

    /** Fills the map which links the string representing of a density unit to the type if the unit. */
    void setDensityUnitMap();

    std::map<std::string, int> m_lengthUnitMap;  /**< Maps a string representing a length unit to the unit type. */
    std::map<std::string, int> m_angleUnitMap;   /**< Maps a string representing an angle unit to the unit type. */
    std::map<std::string, int> m_energyUnitMap;  /**< Maps a string representing an energy unit to the unit type. */
    std::map<std::string, int> m_densityUnitMap; /**< Maps a string representing a density unit to the unit type. */


  private:

    /**
     * The constructor is hidden to avoid that someone creates an instance of this class.
     */
    Unit();

    /** Disable/Hide the copy constructor. */
    Unit(const Unit&);

    /** Disable/Hide the copy assignment operator. */
    Unit& operator=(const Unit&);

    /** The Unit destructor. */
    ~Unit();

    static Unit* m_instance; /**< Pointer that saves the instance of this class. */

    /** Destroyer class to delete the instance of the Gearbox class when the program terminates. */
    class SingletonDestroyer {
    public: ~SingletonDestroyer() {
        if (Unit::m_instance != NULL) delete Unit::m_instance;
      }
    };
    friend class SingletonDestroyer;


  };

}

#endif /* UNIT_H_ */
