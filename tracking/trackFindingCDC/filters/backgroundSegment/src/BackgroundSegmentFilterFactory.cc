/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/backgroundSegment/BackgroundSegmentFilterFactory.h>

#include <tracking/trackFindingCDC/filters/backgroundSegment/BackgroundSegmentTruthVarSet.h>

#include <tracking/trackFindingCDC/filters/segment/AdvancedSegmentVarSet.h>

#include <tracking/trackFindingCDC/filters/base/MVAFilter.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>
#include <tracking/trackFindingCDC/filters/base/MCFilter.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  using AllBackgroundSegmentFilter = AllFilter<BaseBackgroundSegmentFilter>;
  using MCBackgroundSegmentFilter =
    MCFilter<VariadicUnionVarSet<BackgroundSegmentTruthVarSet, AdvancedSegmentVarSet>>;
  using RecordingBackgroundSegmentFilter =
    RecordingFilter<VariadicUnionVarSet<BackgroundSegmentTruthVarSet, AdvancedSegmentVarSet>>;
  using MVABackgroundSegmentFilter = MVAFilter<AdvancedSegmentVarSet>;
}

BackgroundSegmentFilterFactory::BackgroundSegmentFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string BackgroundSegmentFilterFactory::getIdentifier() const
{
  return "BackgroundSegment";
}

std::string BackgroundSegmentFilterFactory::getFilterPurpose() const
{
  return "Segment filter to reject background";
}

std::map<std::string, std::string>
BackgroundSegmentFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no segment track combination is valid"},
    {"all", "filter out all segments as background"},
    {"truth", "monte carlo truth"},
    {"recording", "record variables to a TTree"},
    {"mva", "test with a mva method"}
  };
}

std::unique_ptr<BaseBackgroundSegmentFilter>
BackgroundSegmentFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return makeUnique<BaseBackgroundSegmentFilter>();
  } else if (filterName == "all") {
    return makeUnique<AllBackgroundSegmentFilter>();
  } else if (filterName == "truth") {
    return makeUnique<MCBackgroundSegmentFilter>();
  } else if (filterName == "recording") {
    return makeUnique<RecordingBackgroundSegmentFilter>("BackgroundSegmentFilter.root");
  } else if (filterName == "mva") {
    return makeUnique<MVABackgroundSegmentFilter>("tracking/data/trackfindingcdc_BackgroundSegmentFilter.xml");
  } else {
    return Super::create(filterName);
  }
}
