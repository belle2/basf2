/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentTrain/SegmentTrainFilterFactory.h>
#include <tracking/trackFindingCDC/filters/segmentTrain/SimpleSegmentTrainFilter.h>

#include <tracking/trackFindingCDC/filters/segmentTrain/SegmentTrainTruthVarSet.h>
#include <tracking/trackFindingCDC/filters/segmentTrain/SegmentTrainVarSet.h>

#include <tracking/trackFindingCDC/filters/base/AllFilter.h>
#include <tracking/trackFindingCDC/filters/base/MCFilter.h>
#include <tracking/trackFindingCDC/filters/base/MVAFilter.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  using AllSegmentTrainFilter = AllFilter<BaseSegmentTrainFilter>;
  using MCSegmentTrainFilter = MCFilter<VariadicUnionVarSet<SegmentTrainTruthVarSet, SegmentTrainVarSet>>;
  using RecordingSegmentTrainFilter = RecordingFilter<VariadicUnionVarSet<SegmentTrainTruthVarSet, SegmentTrainVarSet>>;
  using MVASegmentTrainFilter = MVAFilter<SegmentTrainVarSet>;
}

SegmentTrainFilterFactory::SegmentTrainFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string SegmentTrainFilterFactory::getIdentifier() const
{
  return "SegmentTrain";
}

std::string SegmentTrainFilterFactory::getFilterPurpose() const
{
  return "Segment train filter to be used during the construction of segment trains";
}

std::map<std::string, std::string>
SegmentTrainFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no segment track combination is valid"},
    {"truth", "monte carlo truth"},
    {"simple", "mc free with simple criteria"},
    {"recording", "record variables to a TTree"},
    {"mva", "test with a mva method"}
  };
}

std::unique_ptr<BaseSegmentTrainFilter>
SegmentTrainFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return makeUnique<BaseSegmentTrainFilter>();
  } else if (filterName == "truth") {
    return makeUnique<MCSegmentTrainFilter>();
  } else if (filterName == "simple") {
    return makeUnique<SimpleSegmentTrainFilter>();
  } else if (filterName == "recording") {
    return makeUnique<RecordingSegmentTrainFilter>();
  } else if (filterName == "mva") {
    return makeUnique<MVASegmentTrainFilter>();
  } else {
    return Super::create(filterName);
  }
}
