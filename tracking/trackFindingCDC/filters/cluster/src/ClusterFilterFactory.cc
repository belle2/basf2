/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/cluster/ClusterFilterFactory.h>

#include <tracking/trackFindingCDC/filters/cluster/BaseClusterFilter.h>
#include <tracking/trackFindingCDC/filters/cluster/AllClusterFilter.h>
#include <tracking/trackFindingCDC/filters/cluster/UnionRecordingClusterFilter.h>
#include <tracking/trackFindingCDC/filters/cluster/MVABackgroundClusterFilter.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>


using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::FilterFactory<BaseClusterFilter>;

ClusterFilterFactory::ClusterFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

ClusterFilterFactory::~ClusterFilterFactory() = default;

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
    return std::make_unique<AllClusterFilter>();
  } else if (filterName == "unionrecording") {
    return std::make_unique<UnionRecordingClusterFilter>();
  } else if (filterName == "mva_bkg") {
    return std::make_unique<MVABackgroundClusterFilter>();
  } else {
    return Super::create(filterName);
  }
}
