/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ilya Komarov                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/dbobjects/ParticleWeightingKeyMap.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

void ParticleWeightingKeyMap::addAxis(std::string name)
{
  ParticleWeightingAxis* axis = new ParticleWeightingAxis();;
  axis->setName(name);
  // Note: map is sorted by keys (C++ standards)
  m_axes.insert(std::make_pair(name, axis));
}


int ParticleWeightingKeyMap::addKey(NDBin bin, int key_ID)
{
  // Note: it is only possible to add axes to an empty key map
  if (key_ID == m_outOfRangeBin) {
    B2FATAL("You are trying create bin with ID identical to out-of-range Bin : " << m_outOfRangeBin);
    return -1;
  }

  if (m_axes.size() == 0) {
    for (auto i_1dbin : bin) {
      this->addAxis(i_1dbin.first);
    }
  }

  if (bin.size() != m_axes.size()) {
    B2FATAL("Inconsistent dimensionality of added bin");
    return -1;
  }

  std::vector<int> bin_id_collection;
  for (auto i_axis : m_axes) {
    auto it =  bin.find(i_axis.first);
    if (it != bin.end()) {
      bin_id_collection.push_back(i_axis.second->addBin(it->second));
    } else {
      B2FATAL("Names of bin and existing axes don't match");
      return -1;
    }
  }

  for (auto i_bin : m_bins) {
    if (std::equal(i_bin.first.begin(), i_bin.first.end(), bin_id_collection.begin())) {
      B2FATAL("You tried to overwrite existing bin " + std::to_string(i_bin.second) + " with new ID " + std::to_string(key_ID));
      return -1;
    }
  }

  m_bins.push_back(std::make_pair(bin_id_collection, key_ID));
  return key_ID;
}


double ParticleWeightingKeyMap::addKey(NDBin bin)
{
  return this->addKey(bin, m_bins.size());
}


double ParticleWeightingKeyMap::getKey(std::map<std::string, double> values) const
{
  if (values.size() != m_axes.size()) {
    B2FATAL("Inconsistent dimensionality of requested value map");
    return -1;
  }
  std::vector<int> bin_id_collection;
  for (auto i_axis : m_axes) {
    auto it =  values.find(i_axis.first);
    if (it != values.end()) {
      bin_id_collection.push_back(i_axis.second->findBin(it->second));
    } else {
      B2FATAL("Names of bin and existing axes don't match");
      return -1;
    }
  }
  for (auto i_bin : m_bins) {
    if (std::equal(i_bin.first.begin(), i_bin.first.end(), bin_id_collection.begin())) {
      return i_bin.second;
    }
  }
  return m_outOfRangeBin;
}


std::vector<std::string> ParticleWeightingKeyMap::getNames() const
{
  std::vector<std::string> names;
  for (auto i_axis : m_axes) {
    names.push_back(i_axis.first);
  }
  return names;
}


void ParticleWeightingKeyMap::printKeyMap() const
{
  std::string axes_names = "";
  for (auto i_axis : m_axes) {
    axes_names += "'" + i_axis.first + "' bin;";
    i_axis.second->printAxis();
  }
  B2INFO("Bin map \n <" + axes_names + "> : <gobal ID>");
  for (auto i_bin : m_bins) {
    std::string binIDs = "";
    for (auto i_binid : i_bin.first) {
      binIDs += std::to_string(i_binid) + "; ";
    }
    B2INFO(binIDs + " : " + std::to_string(i_bin.second));
  }

}


