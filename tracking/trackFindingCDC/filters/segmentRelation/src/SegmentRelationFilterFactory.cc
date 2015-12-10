/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentRelation/SegmentRelationFilterFactory.h>
#include <tracking/trackFindingCDC/filters/segmentRelation/SegmentRelationFilters.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


FilterFactory<Filter<Relation<CDCRecoSegment2D> > >::FilterFactory(const std::string& defaultFilterName) :
  Super(defaultFilterName)
{
}

std::string FilterFactory<Filter<Relation<CDCRecoSegment2D> > >::getFilterPurpose() const
{
  return "Segment relation filter to be used during the construction of the segment network for in super layer merginig.";
}

std::string FilterFactory<Filter<Relation<CDCRecoSegment2D> > >::getModuleParamPrefix() const
{
  return "SegmentRelation";
}

std::map<std::string, std::string>
FilterFactory<Filter<Relation<CDCRecoSegment2D> > >::getValidFilterNamesAndDescriptions() const
{
  std::map<std::string, std::string>
  filterNames = Super::getValidFilterNamesAndDescriptions();

  filterNames.insert({
    {"none", "no segment relation is valid, stop at segment creation."},
    {"truth", "segment relations from monte carlo truth"},
    {"unionrecording", "record many multiple choosable variable set"},
  });

  return filterNames;
}

std::unique_ptr<Filter<Relation<CDCRecoSegment2D> > >
FilterFactory<Filter<Relation<CDCRecoSegment2D> > >::create(const std::string& filterName) const
{
  if (filterName == string("none")) {
    return std::unique_ptr<Filter<Relation<CDCRecoSegment2D> > >(new BaseSegmentRelationFilter());
  } else if (filterName == string("truth")) {
    return std::unique_ptr<Filter<Relation<CDCRecoSegment2D> > >(new MCSegmentRelationFilter());
  } else if (filterName == string("unionrecording")) {
    return std::unique_ptr<Filter<Relation<CDCRecoSegment2D> > >(new UnionRecordingSegmentRelationFilter());
  } else {
    return Super::create(filterName);
  }
}
