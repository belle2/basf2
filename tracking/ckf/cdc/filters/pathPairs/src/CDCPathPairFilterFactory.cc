/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/cdc/filters/pathPairs/CDCPathPairFilterFactory.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>

#include <tracking/ckf/cdc/filters/pathPairs/DistanceBasedCDCPathPairFilter.h>
#include <tracking/ckf/cdc/filters/pathPairs/ArcLengthBasedCDCPathPairFilter.h>
#include <tracking/ckf/cdc/filters/pathPairs/MCTruthCDCPathPairFilter.h>

#include <tracking/trackFindingCDC/filters/base/AllFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/MVAFilter.icc.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

using namespace Belle2;
using namespace TrackFindingCDC;


CDCPathPairFilterFactory::CDCPathPairFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

CDCPathPairFilterFactory::~CDCPathPairFilterFactory() = default;

std::string CDCPathPairFilterFactory::getIdentifier() const
{
  return "CDCCKFPathPair";
}

std::string CDCPathPairFilterFactory::getFilterPurpose() const
{
  return "Order cdc ckf path pairs";
}

std::map<std::string, std::string> CDCPathPairFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no track combination is valid"},
    {"distance", "ordering based on distance"},
    {"arc_length", "ordering based on arc_length"},
    {"mc_truth", "ordering based on mc truth information"},
  };
}

std::unique_ptr<BaseCDCPathPairFilter>
CDCPathPairFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return std::make_unique<TrackFindingCDC::NoneFilter<BaseCDCPathPairFilter>>();
  } else if (filterName == "all") {
    return std::make_unique<TrackFindingCDC::AllFilter<BaseCDCPathPairFilter>>();
  } else if (filterName == "distance") {
    return std::make_unique<DistanceBasedCDCPathPairFilter>();
  } else if (filterName == "arc_length") {
    return std::make_unique<ArcLengthBasedCDCPathPairFilter>();
  } else if (filterName == "mc_truth") {
    return std::make_unique<MCTruthCDCPathPairFilter>();
  } else {
    return Super::create(filterName);
  }
}
