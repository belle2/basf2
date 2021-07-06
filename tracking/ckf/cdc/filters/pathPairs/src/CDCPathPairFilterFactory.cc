/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/cdc/filters/pathPairs/CDCPathPairFilterFactory.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>

#include <tracking/ckf/cdc/filters/pathPairs/DistanceBasedCDCPathPairFilter.h>
#include <tracking/ckf/cdc/filters/pathPairs/DuplicateCDCPathPairFilter.h>
#include <tracking/ckf/cdc/filters/pathPairs/Chi2BasedCDCPathPairFilter.h>
#include <tracking/ckf/cdc/filters/pathPairs/HitDistanceBasedCDCPathPairFilter.h>
#include <tracking/ckf/cdc/filters/pathPairs/ArcLengthBasedCDCPathPairFilter.h>
#include <tracking/ckf/cdc/filters/pathPairs/ArcLengthBasedCDCfromEclPathPairFilter.h>
#include <tracking/ckf/cdc/filters/pathPairs/MCTruthCDCPathPairFilter.h>

#include <tracking/trackFindingCDC/filters/base/AllFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>

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
  return "Order CDC CKF path pairs. ";
}

std::map<std::string, std::string> CDCPathPairFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no track combination is valid"},
    {"all", "set all track combinations as good"},
    {"distance", "ordering based on distance"},
    {"chi2", "ordering based on chi2"},
    {"duplicateHits", "compares last hits of both tracks to check for duplicates"},
    {"hitDistance", "ordering based on hitDistance"},
    {"arc_length", "ordering based on arc_length"},
    {"arc_length_fromEcl", "ordering based on arc_length; track from Ecl seed"},
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
  } else if (filterName == "chi2") {
    return std::make_unique<Chi2BasedCDCPathPairFilter>();
  } else if (filterName == "duplicateHits") {
    return std::make_unique<DuplicateCDCPathPairFilter>();
  } else if (filterName == "hitDistance") {
    return std::make_unique<HitDistanceBasedCDCPathPairFilter>();
  } else if (filterName == "arc_length") {
    return std::make_unique<ArcLengthBasedCDCPathPairFilter>();
  } else if (filterName == "arc_length_fromEcl") {
    return std::make_unique<ArcLengthBasedCDCfromEclPathPairFilter>();
  } else if (filterName == "mc_truth") {
    return std::make_unique<MCTruthCDCPathPairFilter>();
  } else {
    return Super::create(filterName);
  }
}
