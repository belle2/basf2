/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/dbobjects/ParticleWeightingLookUpTable.h>
#include <framework/logging/Logger.h>

#include <utility>

using namespace Belle2;

void ParticleWeightingLookUpTable::addEntry(WeightInfo entryValue, NDBin bin)
{
  int id = m_WeightMap.size();
  this->addEntry(std::move(entryValue), std::move(bin), id);
}

void ParticleWeightingLookUpTable::addEntry(WeightInfo entryValue, NDBin bin, int key_ID)
{
  int id = m_KeyMap.addKey(std::move(bin), key_ID);
  entryValue.insert(std::make_pair("binID", id));
  m_WeightMap.insert(std::make_pair(id, entryValue));
}


void ParticleWeightingLookUpTable::defineOutOfRangeWeight(const WeightInfo& entryValue)
{
  m_WeightMap.insert(std::make_pair(m_OutOfRangeBinID, entryValue));
}


std::vector<std::string> ParticleWeightingLookUpTable::getAxesNames() const
{
  return m_KeyMap.getNames();
}

WeightInfo ParticleWeightingLookUpTable::getInfo(std::map<std::string, double> values) const
{
  int id = m_KeyMap.getKey(std::move(values));
  auto it = m_WeightMap.find(id);
  if (it != m_WeightMap.end()) {
    return  m_WeightMap.at(id);
  } else {
    B2FATAL("Attampt to acccess undeclared bin");
    return m_WeightMap.at(m_OutOfRangeBinID);
  }
}

void ParticleWeightingLookUpTable::printParticleWeightingLookUpTable() const
{
  m_KeyMap.printKeyMap();
  B2INFO("Printing the table");
  for (const auto& entry : m_WeightMap) {
    int key_ID = entry.first;
    WeightInfo info = entry.second;
    std::string bin_info = "";
    std::string bin_id = "";
    for (const auto& line : info) {
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
