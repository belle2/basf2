/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segment/SegmentFilterFactory.h>

#include <tracking/trackFindingCDC/filters/segment/BkgTruthSegmentVarSet.h>

#include <tracking/trackFindingCDC/filters/segment/AdvancedSegmentVarSet.h>

#include <tracking/trackFindingCDC/filters/base/MVAFilter.icc.h>

#include <tracking/trackFindingCDC/filters/base/RecordingFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/TruthVarFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  using AllSegmentFilter = AllFilter<BaseSegmentFilter>;
  using NoneSegmentFilter = NoneFilter<BaseSegmentFilter>;
  using BkgMCSegmentFilter = TruthVarFilter<BkgTruthSegmentVarSet>;
  using BkgRecordingSegmentFilter =
    RecordingFilter<VariadicUnionVarSet<BkgTruthSegmentVarSet, AdvancedSegmentVarSet>>;
  using BkgMVASegmentFilter = MVAFilter<AdvancedSegmentVarSet>;
}

template class TrackFindingCDC::FilterFactory<BaseSegmentFilter>;

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
    return std::make_unique<NoneSegmentFilter>();
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "all") {
    return std::make_unique<AllSegmentFilter>();
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "bkg_truth") {
    return std::make_unique<BkgMCSegmentFilter>();
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "bkg_recording") {
    return std::make_unique<BkgRecordingSegmentFilter>("BackgroundSegmentFilter.root");
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "bkg_mva") {
    return std::make_unique<BkgMVASegmentFilter>("trackfindingcdc_BackgroundSegmentFilter");
  } else {
    return Super::create(filterName);
  }
}
