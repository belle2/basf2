/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010-2011  Belle II Collaboration                         *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Jakob Lettenbichler                                      *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include "../include/SectorFriends.h"
#include "../include/FullSecID.h"
#include <iostream>
#include <boost/foreach.hpp>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Tracking;


void SectorFriends::addValuePair(int aFilter, pair<double, double> values)
{
  B2DEBUG(150, "adding valuePair of Filter " << aFilter << " in filterVector of " << m_filters.size() << " entries")
  if (m_filters.at(aFilter).getType() != -1) {
    m_filters[aFilter].addValuePair(values.first, values.second);
  } else {
    m_filters[aFilter] = Cutoff(aFilter, values);
  }
}

pair<double, double> SectorFriends::exportFilters(int aFilter)
{
  return make_pair(m_filters.at(aFilter).getMinValue(), m_filters.at(aFilter).getMaxValue());
}

const Cutoff* SectorFriends::getCutOff(int aFilter)
{
  if (m_filters.at(aFilter).getType() != -1) {
    return &m_filters[aFilter];
  }
  B2DEBUG(50, " cutoffType  (int/string) " << aFilter << "/" << FilterID().getFilterString(aFilter) << " does not exist within Friend (int/string) " << m_friendName << "/" << FullSecID(m_friendName).getFullSecString() << " of " << m_sectorName << "/" << FullSecID(m_sectorName).getFullSecString() << "!");
  return NULL;
}

void SectorFriends::getSupportedCutoffs(std::vector<int>& supportedCutoffs)
{
//  for (FilterID::filterTypes filter = FilterID::angles3D; filter < FilterID::numFilters; ++filter)
  for (int filter = FilterID::angles3D; filter < FilterID::numFilters; ++filter) {
    if (m_filters.at(filter).getType() != -1) {
      B2DEBUG(1000, " current filter/cutoffType: " << FilterID().getFilterString(filter));
      supportedCutoffs.push_back(filter);
    }
  }
  B2DEBUG(500, "Friend " << m_friendName << "/" << FullSecID(m_friendName).getFullSecString() << " of " << m_sectorName << "/" << FullSecID(m_sectorName).getFullSecString() << ": after boost loop is size of cutOffs: " << supportedCutoffs.size());
}
