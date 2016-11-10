/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/cluster/ClusterFilterFactory.h>
#include <tracking/trackFindingCDC/filters/cluster/ClusterFilters.h>

using namespace Belle2;
using namespace TrackFindingCDC;

ClusterFilterFactory::ClusterFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string ClusterFilterFactory::getIdentifier() const
{
  return "Cluster";
}

std::string ClusterFilterFactory::getFilterPurpose() const
{
  return "Cluster filter to reject background";
}

std::map<std::string, std::string> ClusterFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"all", "all cluster are valid"},
    {"unionrecording", "record many multiple choosable variable set"},
    {"mva_bkg", "test clusters for background with a mva method"},
  };
}

std::unique_ptr<Filter<CDCWireHitCluster> >
ClusterFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "all") {
    return makeUnique<AllClusterFilter>();
  } else if (filterName == "unionrecording") {
    return makeUnique<UnionRecordingClusterFilter>();
  } else if (filterName == "mva_bkg") {
    return makeUnique<MVABackgroundClusterFilter>();
  } else {
    return Super::create(filterName);
  }
}
