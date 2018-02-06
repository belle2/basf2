/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ilya Komarov                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/dataobjects/LookupTable.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

void LookupTable::addEntry(WeightInfo entryValue, NDBin bin)
{
  double id = m_LookupTable.first.addKey(bin);
  m_LookupTable.second.insert(std::pair<double, WeightInfo>(id, entryValue));
}

void LookupTable::addEntry(WeightInfo entryValue, NDBin bin, double key_ID)
{
  double id = m_LookupTable.first.addKey(bin, key_ID);
  m_LookupTable.second.insert(std::pair<double, WeightInfo>(id, entryValue));
}

void LookupTable::defineOutOfRangeWeight(WeightInfo entryValue)
{
  m_LookupTable.second.insert(std::pair<double, WeightInfo>(-1, entryValue));
}


WeightInfo LookupTable::getFirst()
{
  return m_LookupTable.second.begin()->second;
}

// Getting Lookup info for given particle in given event
WeightInfo LookupTable::getInfo(const Particle* p)
{
  double entryKey = m_LookupTable.first.getKey(p);
  if (m_LookupTable.second.find(entryKey) == m_LookupTable.second.end()) {
    if (entryKey == -1) {
      B2ERROR("This particle is out of range of the lookup table, but weights for this region are not defined. Consider call 'defineOutOfRangeWeight()' function.");
    } else {
      B2ERROR("Bin '" << entryKey << "' is defined in KeyMap, but doesn't have any weight info.");
    }
  }
  return m_LookupTable.second.find(entryKey)->second;
}

