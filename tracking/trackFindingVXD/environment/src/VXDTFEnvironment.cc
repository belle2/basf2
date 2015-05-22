#include "tracking/trackFindingVXD/environment/VXDTFEnvironment.h"

#ifndef __CINT__
#include "tracking/trackFindingVXD/environment/VXDTFSegmentFilter.h"
#include <unordered_map>
#endif

using namespace std;
using namespace Belle2;

#ifndef __CINT__
typedef unordered_map< string, VXDTFSegmentFilter*> stringToSegmentFilter_t;
#endif

VXDTFEnvironment::VXDTFEnvironment(): m_SegmentFilters(NULL)
{
#ifndef __CINT__
  m_SegmentFilters = new stringToSegmentFilter_t;
#endif
}

VXDTFEnvironment::~VXDTFEnvironment()
{
#ifndef __CINT__

  auto filters = (stringToSegmentFilter_t*) m_SegmentFilters;
  for (auto& filter : * filters)
    delete filter.second;

#endif
}


const VXDTFSegmentFilter*
VXDTFEnvironment::getSegmentFilter(const std::string& name)
{
#ifndef __CINT__

  auto filters = (stringToSegmentFilter_t*) m_SegmentFilters;

  auto  result = filters->find(name);
  if (result == filters->end())
    return NULL;
  return result->second;

#endif

}

void
VXDTFEnvironment::assignSegmentFilter(const string& name, VXDTFSegmentFilter* theFilter)
{
#ifndef __CINT__
  auto filters = (stringToSegmentFilter_t*) m_SegmentFilters;
  (*filters)[ name ] = theFilter;
#endif
}

ClassImp(VXDTFEnvironment)

