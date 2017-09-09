/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segment/SegmentFilterFactory.h>

#include <tracking/trackFindingCDC/filters/segment/BkgTruthSegmentVarSet.h>

#include <tracking/trackFindingCDC/filters/segment/AdvancedSegmentVarSet.h>

#include <tracking/trackFindingCDC/filters/base/MVAFilter.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>
#include <tracking/trackFindingCDC/filters/base/MCFilter.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  using AllSegmentFilter = AllFilter<BaseSegmentFilter>;
  using NoneSegmentFilter = NoneFilter<BaseSegmentFilter>;
  using BkgMCSegmentFilter =
    MCFilter<VariadicUnionVarSet<BkgTruthSegmentVarSet, AdvancedSegmentVarSet>>;
  using BkgRecordingSegmentFilter =
    RecordingFilter<VariadicUnionVarSet<BkgTruthSegmentVarSet, AdvancedSegmentVarSet>>;
  using BkgMVASegmentFilter = MVAFilter<AdvancedSegmentVarSet>;
}

SegmentFilterFactory::SegmentFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string SegmentFilterFactory::getIdentifier() const
{
  return "Segment";
}

std::string SegmentFilterFactory::getFilterPurpose() const
{
  return "Segment filter to reject fakes";
}

std::map<std::string, std::string>
SegmentFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no segment track combination is valid"},
    {"all", "filter out all segments as background"},
    {"bkg_truth", "monte carlo truth"},
    {"bkg_recording", "record variables to a TTree"},
    {"bkg_mva", "test with a mva method"}
  };
}

std::unique_ptr<BaseSegmentFilter>
SegmentFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return makeUnique<NoneSegmentFilter>();
  } else if (filterName == "all") {
    return makeUnique<AllSegmentFilter>();
  } else if (filterName == "bkg_truth") {
    return makeUnique<BkgMCSegmentFilter>();
  } else if (filterName == "bkg_recording") {
    return makeUnique<BkgRecordingSegmentFilter>("BackgroundSegmentFilter.root");
  } else if (filterName == "bkg_mva") {
    return makeUnique<BkgMVASegmentFilter>("tracking/data/trackfindingcdc_BackgroundSegmentFilter.xml");
  } else {
    return Super::create(filterName);
  }
}
