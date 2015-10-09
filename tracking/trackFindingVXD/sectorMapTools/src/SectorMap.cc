#include "tracking/trackFindingVXD/sectorMapTools/SectorMap.h"

#ifndef __CINT__
#include "tracking/trackFindingVXD/environment/VXDTFFilters.h"
#include <unordered_map>
#endif

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



