/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Heck, Martin Ritter                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <TMath.h>

using namespace Belle2;
using namespace std;

namespace Belle2 {

  map<string, double> Unit::m_conversionFactors;

  /** Define a new Unit given a name for the conversion table */
#define DEFINE_UNIT_NAME(var,value,name) const double Unit::var = Unit::registerConversion(name,value)
  /** Define a new Unit and take the variable name as name for the conversion table */
#define DEFINE_UNIT(var,value) DEFINE_UNIT_NAME(var,value,#var)

// standard units
  DEFINE_UNIT(cm    , 1.); /**< Standard of [length] */
  DEFINE_UNIT(ns    , 1.); /**< Standard of [time] */
  DEFINE_UNIT(rad   , 1.); /**< Standard of [angle] */
  DEFINE_UNIT(GeV   , 1.); /**< Standard of [energy, momentum, mass] */
  DEFINE_UNIT(K     , 1.); /**< Standard of [temperature] */
  DEFINE_UNIT(T     , 1.); /**< Standard of [magnetic field] */
  DEFINE_UNIT(e     , 1.); /**< Standard of [electric charge] */
  DEFINE_UNIT_NAME(g_cm3 , 1., "g/cm3"); /**< Standard of [density] */

// length units
  DEFINE_UNIT(km  , Unit::cm * 1e5);   /**< [kilometers] */
  DEFINE_UNIT(m   , Unit::cm * 1e2);   /**< [meters] */
  DEFINE_UNIT(mm  , Unit::m  * 1e-3);  /**< [millimeters] */
  DEFINE_UNIT(um  , Unit::m  * 1e-6);  /**< [micrometers] */
  DEFINE_UNIT(nm  , Unit::m  * 1e-9);  /**< [nanometers] */
  DEFINE_UNIT(pm  , Unit::m  * 1e-12); /**< [picometers] */
  DEFINE_UNIT(fm  , Unit::m  * 1e-15); /**< [femtometers] */

// area units
  DEFINE_UNIT(m2  , Unit::m * Unit::m);      /**< [square meters] */
  DEFINE_UNIT(cm2 , Unit::cm * Unit::cm);    /**< [square centimeters] */
  DEFINE_UNIT(mm2 , Unit::mm * Unit::mm);    /**< [square millimeters] */

  DEFINE_UNIT(b   , Unit::m2 * 1e-28); /**< [barn] */
  DEFINE_UNIT(mb  , Unit::b  * 1e-3);  /**< [millibarn] */
  DEFINE_UNIT(ub  , Unit::b  * 1e-6);  /**< [microbarn] */
  DEFINE_UNIT(nb  , Unit::b  * 1e-9);  /**< [nanobarn] */
  DEFINE_UNIT(pb  , Unit::b  * 1e-12); /**< [picobarn] */
  DEFINE_UNIT(fb  , Unit::b  * 1e-15); /**< [femtobarn] */
  DEFINE_UNIT(ab  , Unit::b  * 1e-18); /**< [attobarn] */

// volume units
  DEFINE_UNIT(m3  , Unit::m * Unit::m * Unit::m);    /**< [cubic meters] */
  DEFINE_UNIT(cm3 , Unit::cm * Unit::cm * Unit::cm); /**< [cubic centimeters] */
  DEFINE_UNIT(mm3 , Unit::mm * Unit::mm * Unit::mm); /**< [cubic millimeters] */

// time units
  DEFINE_UNIT(s   , Unit::ns * 1e9);   /**< [second] */
  DEFINE_UNIT(ms  , Unit::s  * 1e-3);  /**< [millisecond] */
  DEFINE_UNIT(us  , Unit::s  * 1e-6);  /**< [microsecond] */
  DEFINE_UNIT(ps  , Unit::s  * 1e-12); /**< [picosecond] */
  DEFINE_UNIT(fs  , Unit::s  * 1e-15); /**< [femtosecond] */

// angle units
  DEFINE_UNIT(mrad , Unit::rad * 1e-3);  /**< [millirad] */
  DEFINE_UNIT(deg  , TMath::DegToRad()); /**< degree to radians */

// energy units
  DEFINE_UNIT(eV  , Unit::GeV * 1e-9); /**< [electronvolt] */
  DEFINE_UNIT(keV , Unit::eV  * 1e3);  /**< [kiloelectronvolt] */
  DEFINE_UNIT(MeV , Unit::eV  * 1e6);  /**< [megaelectronvolt] */
  DEFINE_UNIT(TeV , Unit::eV  * 1e12);  /**< [teraelectronvolt] */
  DEFINE_UNIT(J   , 1.0 / TMath::Qe() * Unit::eV); /**< [joule] */
  DEFINE_UNIT(V   , Unit::eV / Unit::e); /**< Standard of [voltage] */

// charge units
  DEFINE_UNIT(C  , 1.0 / TMath::Qe() * Unit::e);
  DEFINE_UNIT(fC , Unit::C / 1.0e15);

// density units
  DEFINE_UNIT_NAME(mg_cm3  , Unit::g_cm3 * 1e-3, "mg/cm3"); /**< [mg/cm^3] */
  DEFINE_UNIT_NAME(kg_cm3  , Unit::g_cm3 * 1e3, "kg/cm3");  /**< [kg/cm^3] */
  DEFINE_UNIT_NAME(g_mm3   , Unit::g_cm3 / Unit::mm3, "g/mm3");  /**< [g/mm^3] */
  DEFINE_UNIT_NAME(mg_mm3  , Unit::mg_cm3 / Unit::mm3, "mg/mm3"); /**< [mg/mm^3] */
  DEFINE_UNIT_NAME(kg_mm3  , Unit::kg_cm3 / Unit::mm3, "kg/mm3"); /**< [kg/mm^3] */

//Various constants
  const double Unit::speedOfLight   = 29.9792458;
  const double Unit::kBoltzmann     = 8.617343 * 1.0e-5 * Unit::eV / Unit::K;
  const double Unit::ehEnergy       = 3.65 * Unit::eV;
  const double Unit::electronMass   = 0.510999 * MeV;
  const double Unit::fineStrConst   = 1.0 / 137.036;
  const double Unit::permSi         = 11.9 * 8.8542 * 1e-18 * Unit::C / Unit::V / Unit::um;
  const double Unit::uTherm         = 0.026 * Unit::V;
  const double Unit::eMobilitySi    = 1415 * Unit::cm2 / Unit::V / Unit::s;

//SuperKEKB and Belle II constants
  const double Unit::ringCircumference = 3016.360049 * Unit::m;


  double Unit::convertValue(double value, const std::string& unitString)
  {
    map<string, double>::const_iterator it = m_conversionFactors.find(unitString);
    if (it == m_conversionFactors.end()) {
      B2ERROR("Could not find conversion factor for unit " << unitString << ", assuming 1.0");
      return value;
    }
    return it->second * value;
  }

  double Unit::registerConversion(const string& name, double value)
  {
    if (!m_conversionFactors.insert(make_pair(name, value)).second) {
      B2ERROR("Conversion factor for unit " << name << " already exists, ignoring");
    }
    return value;
  }

}
