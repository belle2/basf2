/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/filters/cdcTrackSpacePointCombination/CDCVXDTrackCombinationFilterFactory.h>
#include <tracking/ckf/filters/cdcTrackSpacePointCombination/BaseCDCTrackSpacePointCombinationFilter.h>
#include <tracking/ckf/filters/cdcTrackSpacePointCombination/SimpleCDCVXDTrackCombinationFilter.h>
#include <tracking/ckf/filters/cdcTrackSpacePointCombination/Chi2CDCVXDTrackCombinationFilter.h>
#include <tracking/ckf/filters/cdcTrackSpacePointCombination/CDCVXDTrackCombinationTruthVarSet.h>
#include <tracking/ckf/filters/cdcTrackSpacePointCombination/CDCVXDTrackCombinationVarSet.h>

#include <tracking/trackFindingCDC/filters/base/MCFilter.h>
#include <tracking/trackFindingCDC/filters/base/MVAFilter.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>
#include <tracking/trackFindingCDC/filters/base/NamedChoosableVarSetFilter.h>
#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// MC filter for VXD - CDC relations.
  using MCCDCVXDTrackCombinationFilter = MCFilter<CDCVXDTrackCombinationTruthVarSet>;

  /// MC filter for VXD - CDC relations.
  using MCCTruthNumberCDCVXDTrackCombinationFilter = NamedChoosableVarSetFilter<CDCVXDTrackCombinationTruthVarSet>;

  /// Basic recording filter for VXD - CDC relations.
  using RecordingCDCVXDTrackCombinationFilter =
    RecordingFilter<VariadicUnionVarSet<CDCVXDTrackCombinationTruthVarSet, CDCVXDTrackCombinationVarSet>>;

  /// All filter for VXD - CDC relations.
  using AllCDCVXDTrackCombinationFilter = AllFilter<BaseCDCVXDTrackCombinationFilter>;

  /// Filter using a trained MVA method
  using MVACDCVXDTrackCombinationFilter = MVAFilter<CDCVXDTrackCombinationVarSet>;
}

CDCVXDTrackCombinationFilterFactory::CDCVXDTrackCombinationFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string CDCVXDTrackCombinationFilterFactory::getIdentifier() const
{
  return "CDCVXDTrackCombination";
}

std::string CDCVXDTrackCombinationFilterFactory::getFilterPurpose() const
{
  return "Filter out relations between CDC Reco Tracks and Space Points from the SVD on very easy criteria";
}

std::map<std::string, std::string>
CDCVXDTrackCombinationFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no track combination is valid"},
    {"all", "set all track combinations as good"},
    {"recording", "record variables to a TTree"},
    {"truth", "monte carlo truth"},
    {"truth_number", "monte carlo truth returning the number of correct hits"},
    {"simple", "simple filter based on simple parameters"},
    {"chi2", "filter based on a chi2 calculation"},
  };
}

std::unique_ptr<BaseCDCVXDTrackCombinationFilter>
CDCVXDTrackCombinationFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return makeUnique<NoneFilter<BaseCDCVXDTrackCombinationFilter>>();
  } else if (filterName == "all") {
    return makeUnique<AllCDCVXDTrackCombinationFilter >();
  } else if (filterName == "recording") {
    return makeUnique<RecordingCDCVXDTrackCombinationFilter>();
  } else if (filterName == "mva") {
    return makeUnique<MVACDCVXDTrackCombinationFilter>("tracking/data/ckf_CDCVXDTrackCombination.xml");
  } else if (filterName == "truth") {
    return makeUnique<MCCDCVXDTrackCombinationFilter >();
  } else if (filterName == "truth_number") {
    return makeUnique<MCCTruthNumberCDCVXDTrackCombinationFilter>("truth_number_of_correct_hits");
  } else if (filterName == "simple") {
    return makeUnique<SimpleCDCVXDTrackCombinationFilter>();
  } else if (filterName == "chi2") {
    return makeUnique<Chi2CDCVXDTrackCombinationFilter>();
  } else {
    return Super::create(filterName);
  }
}
