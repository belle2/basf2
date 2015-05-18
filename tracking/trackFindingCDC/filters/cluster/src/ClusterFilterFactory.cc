/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include "../include/ClusterFilterFactory.h"
#include "../include/ClusterFilters.h"

#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCBaseModule.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


ClusterFilterFactory::ClusterFilterFactory() :
  FilterFactory<BaseClusterFilter>("all")
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

std::map<std::string, std::string> ClusterFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"all", "all hits are valid"},
    {"tmva", "test clusters for background with a tmva method"},
    {"recording", "record cluster variables to a TTree"}
  };
}

std::unique_ptr<BaseClusterFilter> ClusterFilterFactory::create(const std::string& filterName) const
{
  if (filterName == string("all")) {
    return std::unique_ptr<BaseClusterFilter>(new AllClusterFilter());
  } else if (filterName == string("tmva")) {
    return std::unique_ptr<BaseClusterFilter>(new TMVAClusterFilter());
  } else if (filterName == string("recording")) {
    return std::unique_ptr<BaseClusterFilter>(new RecordingClusterFilter());
  } else {
    return std::unique_ptr<BaseClusterFilter>(nullptr);
  }
}
