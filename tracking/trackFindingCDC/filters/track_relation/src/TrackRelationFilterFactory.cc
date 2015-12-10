/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/track_relation/TrackRelationFilterFactory.h>
#include <tracking/trackFindingCDC/filters/track_relation/TrackRelationFilters.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


FilterFactory<Filter<Relation<CDCTrack> > >::FilterFactory(const std::string& defaultFilterName) :
  Super(defaultFilterName)
{
}

std::string FilterFactory<Filter<Relation<CDCTrack> > >::getFilterPurpose() const
{
  return "Track relation filter to be used during the construction of the track network for in super layer merginig.";
}

std::string FilterFactory<Filter<Relation<CDCTrack> > >::getModuleParamPrefix() const
{
  return "TrackRelation";
}

std::map<std::string, std::string>
FilterFactory<Filter<Relation<CDCTrack> > >::getValidFilterNamesAndDescriptions() const
{
  std::map<std::string, std::string>
  filterNames = Super::getValidFilterNamesAndDescriptions();

  filterNames.insert({
    {"none", "no track relation is valid, stop at track creation."},
    {"truth", "track relations from monte carlo truth"},
    {"unionrecording", "record many multiple choosable variable set"},
  });

  return filterNames;
}

std::unique_ptr<Filter<Relation<CDCTrack> > >
FilterFactory<Filter<Relation<CDCTrack> > >::create(const std::string& filterName) const
{
  if (filterName == string("none")) {
    return std::unique_ptr<Filter<Relation<CDCTrack> > >(new BaseTrackRelationFilter());
  } else if (filterName == string("truth")) {
    return std::unique_ptr<Filter<Relation<CDCTrack> > >(new MCTrackRelationFilter());
  } else if (filterName == string("unionrecording")) {
    return std::unique_ptr<Filter<Relation<CDCTrack> > >(new UnionRecordingTrackRelationFilter());
  } else {
    return Super::create(filterName);
  }
}
