/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef UNITCONVERTER_H_
#define UNITCONVERTER_H_

#include <string>
#include <map>
#include <cmath>

namespace Belle2 {

  /**
   * The UnitConverter class.
   *
   * This class provides methods to convert floating point numbers from a
   * specified unit to the standard unit of the framework.
   * It is designed as a singleton.
   */
  class UnitConverter {

  public:

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
                            c_TeV  /**< energy unit [TeV]. */
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
    static UnitConverter& Instance();


    /**
     * Converts a floating point value to the standard framework unit.
     *
     * @param value The value which should be converted.
     * @param unitType The unit type of the specified value.
     * @param unitString The string of the unit of the specified value.
     * @return The value converted to the specified unit.
     */
    double convertValue(double value, EUnitTypes unitType, const std::string& unitString) const;


  protected:

    /** Fills the map which links the string representing a length unit to the type if the unit. */
    void setLengthUnitMap();

    /** Fills the map which links the string representing an angle unit to the type if the unit. */
    void setAngleUnitMap();

    /** Fills the map which links the string representing an energy unit to the type if the unit. */
    void setEnergyUnitMap();

    /** Fills the map which links the string representing of a density unit to the type if the unit. */
    void setDensityUnitMap();

    /**
     * Converts a length value to the standard framework length unit [cm].
     *
     * @param value The value which should be converted.
     * @param unitString The string of the unit of the specified value.
     * @return The value converted to the specified unit.
     */
    double convertLength(double value, const std::string& unitString) const;

    /**
     * Converts an angle value to the standard framework angle unit [rad].
     *
     * @param value The value which should be converted.
     * @param unitString The string of the unit of the specified value.
     * @return The value converted to the specified unit.
     */
    double convertAngle(double value, const std::string& unitString) const;

    /**
     * Converts an energy value to the standard framework energy unit [GeV].
     *
     * @param value The value which should be converted.
     * @param unitString The string of the unit of the specified value.
     * @return The value converted to the specified unit.
     */
    double convertEnergy(double value, const std::string& unitString) const;

    /**
     * Converts a density value to the standard framework energy unit [g/cm3].
     *
     * @param value The value which should be converted.
     * @param unitString The string of the unit of the specified value.
     * @return The value converted to the specified unit.
     */
    double convertDensity(double value, const std::string& unitString) const;

    std::map<std::string, int> m_lengthUnitMap;  /**< Maps a string representing a length unit to the unit type. */
    std::map<std::string, int> m_angleUnitMap;   /**< Maps a string representing an angle unit to the unit type. */
    std::map<std::string, int> m_energyUnitMap;  /**< Maps a string representing an energy unit to the unit type. */
    std::map<std::string, int> m_densityUnitMap; /**< Maps a string representing a density unit to the unit type. */


  private:

    /**
     * The constructor is hidden to avoid that someone creates an instance of this class.
     */
    UnitConverter();

    /** Disable/Hide the copy constructor. */
    UnitConverter(const UnitConverter&);

    /** Disable/Hide the copy assignment operator. */
    UnitConverter& operator=(const UnitConverter&);

    /** The UnitConverter destructor. */
    ~UnitConverter();

    static UnitConverter* m_instance; /**< Pointer that saves the instance of this class. */

    /** Destroyer class to delete the instance of the Gearbox class when the program terminates. */
    class SingletonDestroyer {
    public: ~SingletonDestroyer() {
        if (UnitConverter::m_instance != NULL) delete UnitConverter::m_instance;
      }
    };
    friend class SingletonDestroyer;

  };

}

#endif /* UNITCONVERTER_H_ */
