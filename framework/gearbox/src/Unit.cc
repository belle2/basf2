/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/Unit.h>

using namespace Belle2;
using namespace std;

Unit* Unit::m_instance = NULL;

// standard units
const double Unit::cm   = 1.; /**< Standard of [length] */
const double Unit::ns   = 1.; /**< Standard of [time] */
const double Unit::rad  = 1.; /**< Standard of [angle] */
const double Unit::GeV  = 1.; /**< Standard of [energy, momentum, mass] */
const double Unit::K    = 1.; /**< Standard of [temperature] */
const double Unit::T    = 1.; /**< Standard of [magnetic field] */
const double Unit::e    = 1.; /**< Standard of [electric charge] */
const double Unit::gcm3 = 1.; /**< Standard of [density] */

// length units
const double Unit::km  = Unit::cm * 1e5;   /**< [kilometers] */
const double Unit::m   = Unit::cm * 1e2;   /**< [meters] */
const double Unit::mm  = Unit::m  * 1e-3;  /**< [millimeters] */
const double Unit::um  = Unit::m  * 1e-6;  /**< [micrometers] */
const double Unit::nm  = Unit::m  * 1e-9;  /**< [nanometers] */
const double Unit::fm  = Unit::m  * 1e-12; /**< [femtometers] */

// area units
const double Unit::m2  = Unit::m * Unit::m;      /**< [square meters] */
const double Unit::cm2 = Unit::cm * Unit::cm;    /**< [square centimeters] */
const double Unit::mm2 = Unit::mm * Unit::mm;    /**< [square millimeters] */

const double Unit::b   = Unit::m2 * 1e-28; /**< [barn] */
const double Unit::mb  = Unit::b  * 1e-3;  /**< [millibarn] */
const double Unit::ub  = Unit::b  * 1e-6;  /**< [microbarn] */
const double Unit::nb  = Unit::b  * 1e-9;  /**< [nanobarn] */
const double Unit::pb  = Unit::b  * 1e-12; /**< [picobarn] */
const double Unit::fb  = Unit::b  * 1e-15; /**< [femtobarn] */
const double Unit::ab  = Unit::b  * 1e-18; /**< [atobarn] */

// volume units
const double Unit::m3  = Unit::m * Unit::m * Unit::m;    /**< [cubic meters] */
const double Unit::cm3 = Unit::cm * Unit::cm * Unit::cm; /**< [cubic centimeters] */
const double Unit::mm3 = Unit::mm * Unit::mm * Unit::mm; /**< [cubic millimeters] */

// time units
const double Unit::s   = Unit::ns * 1e9;   /**< [second] */
const double Unit::ms  = Unit::s  * 1e-3;  /**< [millisecond] */
const double Unit::us  = Unit::s  * 1e-6;  /**< [microsecond] */
const double Unit::ps  = Unit::s  * 1e-12; /**< [picosecond] */
const double Unit::fs  = Unit::s  * 1e-15; /**< [femtosecond] */

// angle units
const double Unit::mrad = Unit::rad * 1e-3;  /**< [millirad] */
const double Unit::deg  = TMath::DegToRad(); /**< degree to radians */

// energy units
const double Unit::eV  = Unit::GeV * 1e-9; /**< [electronvolt] */
const double Unit::keV = Unit::eV  * 1e3;  /**< [kiloelectronvolt] */
const double Unit::MeV = Unit::eV  * 1e6;  /**< [megaelectronvolt] */
const double Unit::TeV = Unit::eV  * 1e9;  /**< [megaelectronvolt] */
const double Unit::J   = 1.0 / TMath::Qe() * Unit::eV; /**< [joule] */

// charge units
const double Unit::C  = 1.0 / TMath::Qe() * Unit::e;
const double Unit::fC = Unit::C / 1.0e15;

// density units
const double Unit::mgcm3  = Unit::gcm3 * 1e-3; /**< [mg/cm^3] */
const double Unit::kgcm3  = Unit::gcm3 * 1e3;  /**< [kg/cm^3] */
const double Unit::gmm3   = Unit::gcm3 / Unit::mm3;  /**< [g/mm^3] */
const double Unit::mgmm3  = Unit::mgcm3 / Unit::mm3; /**< [mg/mm^3] */
const double Unit::kgmm3  = Unit::kgcm3 / Unit::mm3; /**< [kg/mm^3] */

//Various constants
double Unit::speed_of_light       = 29.9792458; /**< [cm/ns] */
const double Unit::speedOfLight   = 29.9792458; /**< [cm/ns] */
double Unit::k_boltzmann          = 8.617343 * 1.0e-5 * Unit::eV / Unit::K; /**< Boltzmann constant [GeV/K] */
const double Unit::kBoltzmann     = 8.617343 * 1.0e-5 * Unit::eV / Unit::K; /**< Boltzmann constant [GeV/K] */
double Unit::energy_eh            = 3.65 * Unit::eV;  /**< Energy needed to create an electron-hole pair in Si at std. T */
const double Unit::ehEnergy       = 3.65 * Unit::eV;  /**< Energy needed to create an electron-hole pair in Si at std. T */
double Unit::electron_mass        = 0.510999 * MeV;   /**< Electron mass */
const double Unit::electronMass   = 0.510999 * MeV;   /**< Electron mass */
double Unit::fine_str_const       = 1.0 / 137.036;    /**< The fine structure constant */
const double Unit::fineStrConst = 1.0 / 137.036;    /**< The fine structure constant */


Unit& Unit::Instance()
{
  static SingletonDestroyer siDestroyer;
  if (!m_instance) {
    m_instance = new Unit();
  }
  return *m_instance;
}


double Unit::convertValue(double value, EUnitTypes unitType, const std::string& unitString)
{
  switch (unitType) {
    case c_UnitLength:
      value = convertLength(value, unitString);
      break;
    case c_UnitAngle:
      value = convertAngle(value, unitString);
      break;
    case c_UnitEnergy:
      value = convertEnergy(value, unitString);
      break;
    case c_UnitDensity:
      value = convertDensity(value, unitString);
      break;
  }
  return value;
}


double Unit::convertLength(double value, const std::string& unitString)
{
  Unit& unitRef = Instance();

  map<string, int>::const_iterator mapIter;
  ELengthUnitTypes unit = c_CM;

  mapIter = unitRef.m_lengthUnitMap.find(unitString);
  if (mapIter != unitRef.m_lengthUnitMap.end()) unit = static_cast<ELengthUnitTypes>(mapIter->second);

  switch (unit) {
    case c_UM:
      value *= um;
      break;
    case c_MM:
      value *= mm;
      break;
    case c_CM:
      value *= cm;
      break;
    case c_M:
      value *= m;
      break;
    case c_KM:
      value *= km;
      break;
  }
  return value;
}


double Unit::convertAngle(double value, const std::string& unitString)
{
  Unit& unitRef = Instance();

  map<string, int>::const_iterator mapIter;
  EAngleUnitTypes unit = c_Rad;

  mapIter = unitRef.m_angleUnitMap.find(unitString);
  if (mapIter != unitRef.m_angleUnitMap.end()) unit = static_cast<EAngleUnitTypes>(mapIter->second);

  switch (unit) {
    case c_Deg:
      value *= deg;
      break;
    case c_Rad:
      value *= rad;
      break;
    case c_MRad:
      value *= mrad;
      break;
  }
  return value;
}


double Unit::convertEnergy(double value, const std::string& unitString)
{
  Unit& unitRef = Instance();

  map<string, int>::const_iterator mapIter;
  EEnergyUnitTypes unit = c_GeV;

  mapIter = unitRef.m_energyUnitMap.find(unitString);
  if (mapIter != unitRef.m_energyUnitMap.end()) unit = static_cast<EEnergyUnitTypes>(mapIter->second);

  switch (unit) {
    case c_eV:
      value *= eV;
      break;
    case c_keV:
      value *= keV;
      break;
    case c_MeV:
      value *= MeV;
      break;
    case c_GeV:
      value *= GeV;
      break;
    case c_TeV:
      value *= TeV;
      break;
    case c_Joule:
      value *= J;
      break;
  }
  return value;
}


double Unit::convertDensity(double value, const std::string& unitString)
{
  Unit& unitRef = Instance();

  map<string, int>::const_iterator mapIter;
  EDensityUnitTypes unit = c_GCM;

  mapIter = unitRef.m_densityUnitMap.find(unitString);
  if (mapIter != unitRef.m_densityUnitMap.end()) unit = static_cast<EDensityUnitTypes>(mapIter->second);

  switch (unit) {
    case c_GCM:
      value *= gcm3;
      break;
    case c_MGCM:
      value *= mgcm3;
      break;
    case c_KGCM:
      value *= kgcm3;
      break;
    case c_GMM:
      value *= gmm3;
      break;
    case c_MGMM:
      value *= mgmm3;
      break;
    case c_KGMM:
      value *= kgmm3;
      break;
  }
  return value;
}


//============================================================================
//                            Protected methods
//============================================================================

void Unit::setLengthUnitMap()
{
  m_lengthUnitMap.clear();
  m_lengthUnitMap.insert(make_pair("um", c_UM)); //Micrometer
  m_lengthUnitMap.insert(make_pair("mm", c_MM)); //Millimeter
  m_lengthUnitMap.insert(make_pair("cm", c_CM)); //Centimeter
  m_lengthUnitMap.insert(make_pair("m", c_M));   //Meter
  m_lengthUnitMap.insert(make_pair("km", c_KM)); //Kilometer
}


void Unit::setAngleUnitMap()
{
  m_angleUnitMap.clear();
  m_angleUnitMap.insert(make_pair("deg", c_Deg));   //Degree
  m_angleUnitMap.insert(make_pair("rad", c_Rad));   //Radian
  m_angleUnitMap.insert(make_pair("mrad", c_MRad)); //milliradian
}


void Unit::setEnergyUnitMap()
{
  m_energyUnitMap.clear();
  m_energyUnitMap.insert(make_pair("eV", c_eV));   //Electron volt
  m_energyUnitMap.insert(make_pair("keV", c_keV)); //Kiloelectron volt
  m_energyUnitMap.insert(make_pair("MeV", c_MeV)); //Megaelectron volt
  m_energyUnitMap.insert(make_pair("GeV", c_GeV)); //Gigaelectron volt
  m_energyUnitMap.insert(make_pair("TeV", c_TeV)); //Teraelectron volt
}


void Unit::setDensityUnitMap()
{
  m_densityUnitMap.clear();
  m_densityUnitMap.insert(make_pair("g/cm3", c_GCM));   // g/cm3
  m_densityUnitMap.insert(make_pair("mg/cm3", c_MGCM)); // mg/cm3
  m_densityUnitMap.insert(make_pair("kg/cm3", c_KGCM)); // kg/cm3
  m_densityUnitMap.insert(make_pair("g/mm3", c_GMM));   // g/mm3
  m_densityUnitMap.insert(make_pair("mg/mm3", c_MGMM)); // mg/mm3
  m_densityUnitMap.insert(make_pair("kg/mm3", c_KGMM)); // kg/mm3
}


//============================================================================
//                              Private methods
//============================================================================

Unit::Unit()
{
  setLengthUnitMap();
  setAngleUnitMap();
  setEnergyUnitMap();
  setDensityUnitMap();
}


Unit::~Unit()
{

}
