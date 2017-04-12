/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/cdcToVXDExtrapolator/CDCTrackSpacePointCombinationFilterFactory.h>
#include <tracking/modules/cdcToVXDExtrapolator/BaseCDCTrackSpacePointCombinationFilter.h>

#include <tracking/modules/cdcToVXDExtrapolator/CDCTrackSpacePointCombinationTruthVarSet.h>
#include <tracking/modules/cdcToVXDExtrapolator/CDCTrackSpacePointCombinationVarSet.h>
#include <tracking/modules/cdcToVXDExtrapolator/SimpleCDCToVXDExtrapolationFilter.h>

#include <tracking/trackFindingCDC/filters/base/Filter.h>
#include <tracking/trackFindingCDC/filters/base/MCFilter.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.h>
#include <tracking/trackFindingCDC/filters/base/PassThroughFilter.h>
#include <tracking/trackFindingCDC/filters/base/MVAFilter.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>
#include <tracking/trackFindingCDC/filters/base/RandomFilter.h>
#include <tracking/trackFindingCDC/filters/base/NamedChoosableVarSetFilter.h>
#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// MC filter for VXD - CDC relations.
  using MCCDCTrackSpacePointCombinationFilter =
    MCFilter<VariadicUnionVarSet<CDCTrackSpacePointCombinationTruthVarSet, CDCTrackSpacePointCombinationVarSet>>;

  /// Recording filter for VXD - CDC relations.
  using RecordingCDCTrackSpacePointCombinationFilter =
    RecordingFilter<VariadicUnionVarSet<CDCTrackSpacePointCombinationTruthVarSet, CDCTrackSpacePointCombinationVarSet>>;

  /// All filter for VXD - CDC relations.
  using AllCDCTrackSpacePointCombinationFilter = AllFilter<BaseCDCTrackSpacePointCombinationFilter>;

  /// MVA filter for VXD - CDC relations.
  using MVACDCTrackSpacePointCombinationFilter = MVAFilter<CDCTrackSpacePointCombinationVarSet>;
}

CDCTrackSpacePointCombinationFilterFactory::CDCTrackSpacePointCombinationFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string CDCTrackSpacePointCombinationFilterFactory::getIdentifier() const
{
  return "CDCTrackSpacePointCombination";
}

std::string CDCTrackSpacePointCombinationFilterFactory::getFilterPurpose() const
{
  return "Filter out relations between CDC Reco Tracks and Space Points from the SVD on very easy criteria";
}

std::map<std::string, std::string>
CDCTrackSpacePointCombinationFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no track combination is valid"},
    {"all", "set all track combinations as good"},
    {"simple", "based on non-extrapolation variables"},
    {"truth", "monte carlo truth"},
    {"recording", "record variables to a TTree"},
    {"mva", "test with a mva method"},
  };
}

std::unique_ptr<BaseCDCTrackSpacePointCombinationFilter>
CDCTrackSpacePointCombinationFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return makeUnique<BaseCDCTrackSpacePointCombinationFilter>();
  } else if (filterName == "all") {
    return makeUnique<AllCDCTrackSpacePointCombinationFilter>();
  } else if (filterName == "simple") {
    return makeUnique<SimpleCDCToVXDExtrapolationFilter>();
  } else if (filterName == "truth") {
    return makeUnique<MCCDCTrackSpacePointCombinationFilter>();
  } else if (filterName == "recording") {
    return makeUnique<RecordingCDCTrackSpacePointCombinationFilter>("CDCTrackSpacePointCombinationFilter.root");
  } else if (filterName == "mva") {
    return makeUnique<MVACDCTrackSpacePointCombinationFilter>("tracking/data/vxdcdc_CDCTrackSpacePointCombinationFilter.xml");
  } else {
    return Super::create(filterName);
  }
}
