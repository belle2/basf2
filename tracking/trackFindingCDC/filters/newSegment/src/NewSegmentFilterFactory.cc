/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/newSegment/NewSegmentFilterFactory.h>

#include <tracking/trackFindingCDC/filters/newSegment/NewSegmentTruthVarSet.h>

#include <tracking/trackFindingCDC/filters/segment/AdvancedSegmentVarSet.h>

#include <tracking/trackFindingCDC/filters/base/MVAFilter.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>
#include <tracking/trackFindingCDC/filters/base/MCFilter.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  using AllNewSegmentFilter = AllFilter<BaseNewSegmentFilter>;
  using MCNewSegmentFilter =
    MCFilter<VariadicUnionVarSet<NewSegmentTruthVarSet, AdvancedSegmentVarSet>>;
  using RecordingNewSegmentFilter =
    RecordingFilter<VariadicUnionVarSet<NewSegmentTruthVarSet, AdvancedSegmentVarSet>>;
  using MVANewSegmentFilter = MVAFilter<AdvancedSegmentVarSet>;
}

NewSegmentFilterFactory::NewSegmentFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}
std::string NewSegmentFilterFactory::getIdentifier() const
{
  return "NewSegment";
}

std::string NewSegmentFilterFactory::getFilterPurpose() const
{
  return "Segment background finder.";
}

std::map<std::string, std::string>
NewSegmentFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no segment track combination is valid"},
    {"truth", "monte carlo truth"},
    {"recording", "record variables to a TTree"},
    {"mva", "test with a mva method"}
  };
}

std::unique_ptr<BaseNewSegmentFilter>
NewSegmentFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return makeUnique<BaseNewSegmentFilter>();
  } else if (filterName == "truth") {
    return makeUnique<MCNewSegmentFilter>();
  } else if (filterName == "recording") {
    return makeUnique<RecordingNewSegmentFilter>("NewSegmentFilter.root");
  } else if (filterName == "mva") {
    return makeUnique<MVANewSegmentFilter>("tracking/data/trackfindingcdc_NewSegmentFilter.xml");
  } else {
    return Super::create(filterName);
  }
}
