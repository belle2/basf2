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

ClusterFilterFactory::ClusterFilterFactory(const std::string& defaultFilterName) :
  Super(defaultFilterName)
{
}

std::string ClusterFilterFactory::getFilterPurpose() const
{
  return "Cluster filter examines hit clusters and discards clusters recognised as background.";
}

std::string ClusterFilterFactory::getModuleParamPrefix() const
{
  return "Cluster";
}

std::map<std::string, std::string>
ClusterFilterFactory::getValidFilterNamesAndDescriptions() const
{
  std::map<std::string, std::string>
  filterNames = Super::getValidFilterNamesAndDescriptions();

  filterNames.insert({
    {"all", "all hits are valid"},
    {"tmva", "test clusters for background with a tmva method"},
    {"recording", "record cluster variables to a TTree"},
    {"unionrecording", "record many multiple choosable variable set"},
  });
  return filterNames;
}

std::unique_ptr<Filter<CDCWireHitCluster> >
ClusterFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "all") {
    return std::unique_ptr<Filter<CDCWireHitCluster> >(new AllClusterFilter());
  } else if (filterName == "tmva") {
    return std::unique_ptr<Filter<CDCWireHitCluster> >(new TMVAClusterFilter());
  } else if (filterName == "recording") {
    return std::unique_ptr<Filter<CDCWireHitCluster> >(new RecordingClusterFilter());
  } else if (filterName == "unionrecording") {
    return std::unique_ptr<Filter<CDCWireHitCluster> >(new UnionRecordingClusterFilter());
  } else {
    return Super::create(filterName);
  }
}
