/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/UnitConverter.h>

#include <framework/datastore/Units.h>

using namespace Belle2;
using namespace std;

UnitConverter* UnitConverter::m_instance = NULL;


UnitConverter& UnitConverter::Instance()
{
  static SingletonDestroyer siDestroyer;
  if (!m_instance) {
    m_instance = new UnitConverter();
  }
  return *m_instance;
}


double UnitConverter::convertValue(double value, EUnitTypes unitType, const std::string& unitString) const
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


//============================================================================
//                            Protected methods
//============================================================================

double UnitConverter::convertLength(double value, const std::string& unitString) const
{
  map<string, int>::const_iterator mapIter;
  ELengthUnitTypes unit = c_CM;

  mapIter = m_lengthUnitMap.find(unitString);
  if (mapIter != m_lengthUnitMap.end()) unit = static_cast<ELengthUnitTypes>(mapIter->second);

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


double UnitConverter::convertAngle(double value, const std::string& unitString) const
{
  map<string, int>::const_iterator mapIter;
  EAngleUnitTypes unit = c_Rad;

  mapIter = m_angleUnitMap.find(unitString);
  if (mapIter != m_angleUnitMap.end()) unit = static_cast<EAngleUnitTypes>(mapIter->second);

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


double UnitConverter::convertEnergy(double value, const std::string& unitString) const
{
  map<string, int>::const_iterator mapIter;
  EEnergyUnitTypes unit = c_GeV;

  mapIter = m_energyUnitMap.find(unitString);
  if (mapIter != m_energyUnitMap.end()) unit = static_cast<EEnergyUnitTypes>(mapIter->second);

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
  }
  return value;
}


double UnitConverter::convertDensity(double value, const std::string& unitString) const
{
  map<string, int>::const_iterator mapIter;
  EDensityUnitTypes unit = c_GCM;

  mapIter = m_densityUnitMap.find(unitString);
  if (mapIter != m_densityUnitMap.end()) unit = static_cast<EDensityUnitTypes>(mapIter->second);

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


void UnitConverter::setLengthUnitMap()
{
  m_lengthUnitMap.clear();
  m_lengthUnitMap.insert(make_pair("um", c_UM)); //Micrometer
  m_lengthUnitMap.insert(make_pair("mm", c_MM)); //Millimeter
  m_lengthUnitMap.insert(make_pair("cm", c_CM)); //Centimeter
  m_lengthUnitMap.insert(make_pair("m", c_M));   //Meter
  m_lengthUnitMap.insert(make_pair("km", c_KM)); //Kilometer
}


void UnitConverter::setAngleUnitMap()
{
  m_angleUnitMap.clear();
  m_angleUnitMap.insert(make_pair("deg", c_Deg));   //Degree
  m_angleUnitMap.insert(make_pair("rad", c_Rad));   //Radian
  m_angleUnitMap.insert(make_pair("mrad", c_MRad)); //milliradian
}


void UnitConverter::setEnergyUnitMap()
{
  m_energyUnitMap.clear();
  m_energyUnitMap.insert(make_pair("eV", c_eV));   //Electron volt
  m_energyUnitMap.insert(make_pair("keV", c_keV)); //Kiloelectron volt
  m_energyUnitMap.insert(make_pair("MeV", c_MeV)); //Megaelectron volt
  m_energyUnitMap.insert(make_pair("GeV", c_GeV)); //Gigaelectron volt
  m_energyUnitMap.insert(make_pair("TeV", c_TeV)); //Teraelectron volt
}


void UnitConverter::setDensityUnitMap()
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

UnitConverter::UnitConverter()
{
  setLengthUnitMap();
  setAngleUnitMap();
  setEnergyUnitMap();
  setDensityUnitMap();
}


UnitConverter::~UnitConverter()
{

}
