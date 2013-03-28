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
#include <iostream>
#include <boost/foreach.hpp>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;

void SectorFriends::addValuePair(string cutOffType, pair<double, double> values)
{
  map<string, Cutoff*>::iterator mapIter = m_filterMap.find(cutOffType);
  if (mapIter == m_filterMap.end()) {
    Cutoff* aCutOffPtr =  new Cutoff(cutOffType, values);
    m_filterMap.insert(std::make_pair(cutOffType, aCutOffPtr));
  } else {
    m_filterMap.find(cutOffType)->second->addValuePair(values.first, values.second);
  }
}

pair<double, double> SectorFriends::exportFilters(std::string cutOffType)
{
  pair<double, double> cutOffs;
  cutOffs = make_pair(m_filterMap.find(cutOffType)->second->getMinValue(), m_filterMap.find(cutOffType)->second->getMaxValue());
  return cutOffs;
}

Cutoff* SectorFriends::getCutOff(string cutOffType)
{
  map<string, Cutoff*>::iterator mapIter = m_filterMap.find(cutOffType);
  if (mapIter != m_filterMap.end()) {
    Cutoff* aPtr = m_filterMap.find(cutOffType)->second;
    return aPtr;
  } else {
    B2DEBUG(50, " cutoffType " << cutOffType << " does not exist within Friend " << m_friendName << " of " << m_sectorName << "!");
    return NULL;
  }
}

const vector<string> SectorFriends::getSupportedCutoffs()
{
  vector<string> cutoffs;
  pair<string, Cutoff*> mapEntry;
  BOOST_FOREACH(mapEntry, m_filterMap) {
    B2DEBUG(1000, " current Mapentry.first: " << mapEntry.first);
    cutoffs.push_back(mapEntry.first);
  }
  B2DEBUG(500, "Friend " << m_friendName << " of " << m_sectorName << ": after boost loop is size of cutOffs: " << cutoffs.size());
  return cutoffs;
}
