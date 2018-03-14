/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ilya Komarov                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/dbobjects/LookupTable.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

void LookupTable::addEntry(WeightInfo entryValue, NDBin bin)
{
  B2INFO("Adding entry to lookup table");
  double id = m_LookupTable.first.addKey(bin);
  m_LookupTable.second.insert(std::pair<double, WeightInfo>(id, entryValue));
}

void LookupTable::addEntry(WeightInfo entryValue, NDBin bin, double key_ID)
{
  B2INFO("Adding entry to lookup table with specific key_ID: " << key_ID);
  double id = m_LookupTable.first.addKey(bin, key_ID);
  m_LookupTable.second.insert(std::pair<double, WeightInfo>(id, entryValue));
}

void LookupTable::defineOutOfRangeWeight(WeightInfo entryValue)
{
  B2INFO("Definition of out-of-range weights");
  m_LookupTable.second.insert(std::pair<double, WeightInfo>(m_OutOfRangeBinID, entryValue));
}


WeightInfo LookupTable::getFirst()
{
  B2INFO("Getting first entry of the lookup table");
  return m_LookupTable.second.begin()->second;
}

/*
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
*/

void LookupTable::printLookupTable()
{
  B2INFO("Printing the table");
  for (auto entry : m_LookupTable.second) {
    double key_ID = entry.first;
    WeightInfo info = entry.second;
    if (key_ID == m_OutOfRangeBinID) {
      B2INFO("----- Out Of Range Bin start -----\n");
      for (auto line : info) {
        B2INFO(line.first << ": " << line.second << "\n");
      }
      B2INFO("----- Out Of Range Bin stop ------\n");
    } else {
      NDBin bin = m_LookupTable.first.getNDBin(key_ID);
      B2INFO("----- Bin " << key_ID << " start -----\n");
      for (auto bin1d : bin) {
        B2INFO(bin1d.first << ": [" << bin1d.second.first << "; " << bin1d.second.second << "]\n");
      }
      for (auto line : info) {
        B2INFO(line.first << ": " << line.second << "\n");
      }
      B2INFO("----- Bin " << key_ID << " stop ------\n");
    }
  }
}
