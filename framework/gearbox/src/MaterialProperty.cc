/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/MaterialProperty.h>

using namespace std;
using namespace Belle2;


bool MaterialProperty::addValue(double energy, double value)
{
  //Check if the energy was already used
  map<double, double>::iterator mapIter = m_valueMap.find(energy);

  //If not, add it to the map
  if (mapIter == m_valueMap.end()) {
    m_valueMap.insert(make_pair(energy, value));
    return true;
  } else return false;
}


void MaterialProperty::fillArrays(double* energies, double* values)
{
  int index = 0;
  for (map<double, double>::iterator mapIter = m_valueMap.begin();  mapIter != m_valueMap.end(); mapIter++) {
    energies[index] = mapIter->first;
    values[index] = mapIter->second;
    index++;
  }
}


ClassImp(MaterialProperty)
