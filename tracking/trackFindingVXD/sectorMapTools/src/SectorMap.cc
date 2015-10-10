/********************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                           *
 * Copyright(C) 2015 - Belle II Collaboration                                   *
 *                                                                              *
 * Author: The Belle II Collaboration                                           *
 * Contributors: Eugenio Paoloni                                                *
 *                                                                              *
 * This software is provided "as is" without any warranty.                      *
 *******************************************************************************/

#include "tracking/trackFindingVXD/sectorMapTools/SectorMap.h"

using namespace std;
using namespace Belle2;

#ifndef __CINT__
typedef unordered_map< string, VXDTFFilters*> setupNameToFilters_t;
#endif

SectorMap::SectorMap(): m_allSetupsFilters(NULL)
{
#ifndef __CINT__
  m_allSetupsFilters = new setupNameToFilters_t;
#endif
}

SectorMap::~SectorMap()
{
#ifndef __CINT__

  auto all_filters = (setupNameToFilters_t*) m_allSetupsFilters;
  for (auto& filter : * all_filters)
    delete filter.second;

#endif
}


const VXDTFFilters*
SectorMap::getFilters(const std::string& setupName)
{
#ifndef __CINT__

  auto all_filters = (setupNameToFilters_t*) m_allSetupsFilters;

  auto  result = all_filters->find(setupName);
  if (result == all_filters->end())
    return NULL;
  return result->second;

#endif

}


void
SectorMap::assignFilters(const string& setupName, VXDTFFilters* filters)
{
#ifndef __CINT__
  auto all_filters = (setupNameToFilters_t*) m_allSetupsFilters;
  (*all_filters)[ setupName ] = filters;
#endif
}


#ifndef __CINT__
const unordered_map< string, VXDTFFilters*>&
SectorMap::getAllSetups(void)
{
  return * (setupNameToFilters_t*) m_allSetupsFilters;
}
#endif
