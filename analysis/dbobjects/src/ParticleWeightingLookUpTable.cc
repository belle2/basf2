/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ilya Komarov                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/dbobjects/ParticleWeightingLookUpTable.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

void ParticleWeightingLookUpTable::addEntry(WeightInfo entryValue, NDBin bin)
{
  B2INFO("Adding entry to LookUp table");
  double id = m_ParticleWeightingLookUpTable.first.addKey(bin);
  m_ParticleWeightingLookUpTable.second.insert(std::pair<double, WeightInfo>(id, entryValue));
}

void ParticleWeightingLookUpTable::addEntry(WeightInfo entryValue, NDBin bin, double key_ID)
{
  B2INFO("Adding entry to LookUp table with specific key_ID: " << key_ID);
  double id = m_ParticleWeightingLookUpTable.first.addKey(bin, key_ID);
  m_ParticleWeightingLookUpTable.second.insert(std::pair<double, WeightInfo>(id, entryValue));
}

void ParticleWeightingLookUpTable::defineOutOfRangeWeight(WeightInfo entryValue)
{
  B2INFO("Definition of out-of-range weights");
  m_ParticleWeightingLookUpTable.second.insert(std::pair<double, WeightInfo>(m_OutOfRangeBinID, entryValue));
}


WeightInfo ParticleWeightingLookUpTable::getFirst()
{
  B2INFO("Getting first entry of the LookUp table");
  return m_ParticleWeightingLookUpTable.second.begin()->second;
}

/*
// Getting LookUp info for given particle in given event
WeightInfo ParticleWeightingLookUpTable::getInfo(const Particle* p)
{
  double entryKey = m_ParticleWeightingLookUpTable.first.getKey(p);
  if (m_ParticleWeightingLookUpTable.second.find(entryKey) == m_ParticleWeightingLookUpTable.second.end()) {
    if (entryKey == -1) {
      B2ERROR("This particle is out of range of the LookUp table, but weights for this region are not defined. Consider call 'defineOutOfRangeWeight()' function.");
    } else {
      B2ERROR("Bin '" << entryKey << "' is defined in ParticleWeightingKeyMap, but doesn't have any weight info.");
    }
  }
  return m_ParticleWeightingLookUpTable.second.find(entryKey)->second;
}
*/

void ParticleWeightingLookUpTable::printParticleWeightingLookUpTable()
{
  B2INFO("Printing the table");
  for (auto entry : m_ParticleWeightingLookUpTable.second) {
    double key_ID = entry.first;
    WeightInfo info = entry.second;
    if (key_ID == m_OutOfRangeBinID) {
      B2INFO("----- Out Of Range Bin start -----\n");
      for (auto line : info) {
        B2INFO(line.first << ": " << line.second << "\n");
      }
      B2INFO("----- Out Of Range Bin stop ------\n");
    } else {
      NDBin bin = m_ParticleWeightingLookUpTable.first.getNDBin(key_ID);
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

ParticleWeightingKeyMap ParticleWeightingLookUpTable::getParticleWeightingKeyMap()
{
  return m_ParticleWeightingLookUpTable.first;
}

WeightMap ParticleWeightingLookUpTable::getWeightMap()
{
  return m_ParticleWeightingLookUpTable.second;
}
