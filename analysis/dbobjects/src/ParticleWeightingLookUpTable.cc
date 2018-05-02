/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
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
  int id = m_WeightMap.size();
  this->addEntry(entryValue, bin, id);
}

void ParticleWeightingLookUpTable::addEntry(WeightInfo entryValue, NDBin bin, int key_ID)
{
  int id = m_KeyMap.addKey(bin, key_ID);
  entryValue.insert(std::make_pair("binID", id));
  m_WeightMap.insert(std::make_pair(id, entryValue));
}


void ParticleWeightingLookUpTable::defineOutOfRangeWeight(WeightInfo entryValue)
{
  m_WeightMap.insert(std::make_pair(m_OutOfRangeBinID, entryValue));
}


std::vector<std::string> ParticleWeightingLookUpTable::getAxesNames()
{
  return m_KeyMap.getNames();
}

WeightInfo ParticleWeightingLookUpTable::getInfo(std::map<std::string, double> values)
{
  int id = m_KeyMap.getKey(values);
  auto it = m_WeightMap.find(id);
  if (it != m_WeightMap.end()) {
    return  m_WeightMap.at(id);
  } else {
    B2FATAL("Attampt to acccess undeclared bin");
    return m_WeightMap.at(m_OutOfRangeBinID);
  }
}

void ParticleWeightingLookUpTable::printParticleWeightingLookUpTable()
{
  m_KeyMap.printKeyMap();
  B2INFO("Printing the table");
  for (auto entry : m_WeightMap) {
    int key_ID = entry.first;
    WeightInfo info = entry.second;
    std::string bin_info = "";
    std::string bin_id = "";
    for (auto line : info) {
      bin_info += line.first + " " + std::to_string(line.second) + " ; ";
    }
    if (key_ID == m_OutOfRangeBinID) {
      bin_id += "Out of range bin " + std::to_string(key_ID) + " : ";
    } else {
      bin_id += "Bin " + std::to_string(key_ID) + " : ";
    }
    B2INFO(bin_id + bin_info);
  }
}