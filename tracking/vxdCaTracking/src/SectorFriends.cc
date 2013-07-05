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

SectorFriends::SectorFriends(unsigned int myName, unsigned int secName):
  m_friendName(myName),
  m_sectorName(secName)
{
  m_filters.assign(FilterID::numFilters, NULL);
}

void SectorFriends::addValuePair(int aFilter, pair<double, double> values)
{
  if (m_filters[aFilter] != NULL) {
    m_filters[aFilter]->addValuePair(values.first, values.second);
  } else {
    Cutoff* aCutOffPtr =  new Cutoff(aFilter, values);
    m_filters[aFilter] = aCutOffPtr;
  }
}

pair<double, double> SectorFriends::exportFilters(int aFilter)
{
  return make_pair(m_filters[aFilter]->getMinValue(), m_filters[aFilter]->getMaxValue());
}

Cutoff* SectorFriends::getCutOff(int aFilter)
{
  if (m_filters[aFilter] != NULL) {
    return m_filters[aFilter];
  } else {
    B2DEBUG(50, " cutoffType  (int/string) " << aFilter << "/" << FilterID().getFilterString(aFilter) << " does not exist within Friend (int/string) " << m_friendName << "/" << FullSecID(m_friendName).getFullSecString() << " of " << m_sectorName << "/" << FullSecID(m_sectorName).getFullSecString() << "!");
    return NULL;
  }
}

void SectorFriends::getSupportedCutoffs(std::vector<int>& supportedCutoffs)
{
//  for (FilterID::filterTypes filter = FilterID::angles3D; filter < FilterID::numFilters; ++filter)
  for (int filter = FilterID::angles3D; filter < FilterID::numFilters; ++filter) {
    if (m_filters[filter] != NULL) {
      B2DEBUG(1000, " current filter/cutoffType: " << FilterID().getFilterString(filter));
      supportedCutoffs.push_back(filter);
    }
  }
  B2DEBUG(500, "Friend " << m_friendName << "/" << FullSecID(m_friendName).getFullSecString() << " of " << m_sectorName << "/" << FullSecID(m_sectorName).getFullSecString() << ": after boost loop is size of cutOffs: " << supportedCutoffs.size());
}
