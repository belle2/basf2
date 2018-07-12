/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ilya Komarov                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/dbobjects/ParticleWeightingAxis.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

bool ParticleWeightingAxis::isOverlappingBin(ParticleWeightingBinLimits* bin)
{
  for (auto i_bin : m_unnamedAxis) {
    /**
     * Checkin gif lower border is within some bin
     * existing binning:     |   |
     * new binning:            |     |
     */
    if ((bin->first() >= i_bin.second->first()) and (bin->first() < i_bin.second->second())) {
      return true;
    }
    /**
     * Checking if upper border is within some bin
     * existing binning:          |   |
     * new binning:            |     |
     */
    if ((bin->second() > i_bin.second->first()) and (bin->second() <= i_bin.second->second())) {
      return true;
    }
    /**
     * Checking if new bin covers exiting bin completely
     * existing binning:          |   |
     * new binning:            |        |
     */
    if ((bin->first() < i_bin.second->first()) and (bin->second() > i_bin.second->second())) {
      return true;
    }
  }
  return false;
}


int ParticleWeightingAxis::addBin(ParticleWeightingBinLimits* bin)
{
  int existing_id = this->findBin(bin);
  if (existing_id != m_outOfRangeBinID) {
    return existing_id;
  }
  if (this->isOverlappingBin(bin)) {
    B2FATAL("Attepting to add overlapping or existing bin");
  } else {
    int id = m_unnamedAxis.size() + 1;
    m_unnamedAxis.insert(std::make_pair(id, bin));
    return id;
  }
}


int ParticleWeightingAxis::findBin(ParticleWeightingBinLimits* bin)
{
  for (auto i_bin : m_unnamedAxis) {
    if ((bin->first() == i_bin.second->first()) and (bin->second() == i_bin.second->second())) {
      return i_bin.first;
    }
  }
  return m_outOfRangeBinID;
}


int ParticleWeightingAxis::findBin(double value)
{
  for (auto i_bin : m_unnamedAxis) {
    if ((value >= i_bin.second->first()) and (value < i_bin.second->second())) {
      return i_bin.first;
    }
  }
  return m_outOfRangeBinID;
}

void ParticleWeightingAxis::printAxis()
{
  B2INFO("Printing axis " + m_axisName);
  int len = 10;
  std::string id_cells = "";
  std::string bin_cells = "";
  for (auto i_entry : m_unnamedAxis) {
    std::string i_id = std::to_string(i_entry.first);
    int i_id_len = i_id.size();
    std::string i_ll = std::to_string(i_entry.second->first());
    int i_ll_len = i_ll.size();
    std::string i_ul = std::to_string(i_entry.second->second());
    int i_ul_len = i_ul.size();
    id_cells += "|" + std::string(len, ' ') + i_id + std::string(len - i_id_len + 1, ' ') + "|";
    bin_cells += "|" + i_ll + std::string(len - i_ll_len, ' ') + "|" + i_ul + std::string(len - i_ul_len, ' ') + "|";
  }
  B2INFO(id_cells);
  B2INFO(bin_cells);
}
