/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentInformationListTrack/SegmentInformationListTrackFilterFactory.h>
#include <tracking/trackFindingCDC/filters/segmentInformationListTrack/SimpleSegmentInformationListTrackFilter.h>

#include <tracking/trackFindingCDC/filters/segmentTrain/SegmentTrainTruthVarSet.h>
#include <tracking/trackFindingCDC/filters/segmentTrain/SegmentTrainVarSet.h>

#include <tracking/trackFindingCDC/filters/base/TMVAFilter.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>
#include <tracking/trackFindingCDC/filters/base/MCFilter.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  using AllSegmentInformationListTrackFilter = AllFilter<BaseSegmentInformationListTrackFilter>;
  using MCSegmentInformationListTrackFilter = MCFilter<VariadicUnionVarSet<SegmentTrainTruthVarSet, SegmentTrainVarSet>>;
  using RecordingSegmentInformationListTrackFilter =
    RecordingFilter<VariadicUnionVarSet<SegmentTrainTruthVarSet, SegmentTrainVarSet>>;
  using TMVASegmentInformationListTrackFilter = TMVAFilter<SegmentTrainVarSet>;
}

SegmentInformationListTrackFilterFactory::SegmentInformationListTrackFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string SegmentInformationListTrackFilterFactory::getIdentifier() const
{
  return "SegmentInformationListTrack";
}

std::string SegmentInformationListTrackFilterFactory::getFilterPurpose() const
{
  return "Segment information list to track filter for combinatoric adding of segments to tracks";
}

std::map<std::string, std::string>
SegmentInformationListTrackFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no segment track combination is valid"},
    {"truth", "monte carlo truth"},
    {"simple", "mc free with simple criteria"},
    {"recording", "Record to a ttree"},
    {"tmva", "test using tmva methods"},
  };
}

std::unique_ptr<BaseSegmentInformationListTrackFilter>
SegmentInformationListTrackFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return makeUnique<BaseSegmentInformationListTrackFilter>();
  } else if (filterName == "truth") {
    return makeUnique<MCSegmentInformationListTrackFilter>();
  } else if (filterName == "simple") {
    return makeUnique<SimpleSegmentInformationListTrackFilter>();
  } else if (filterName == "recording") {
    return makeUnique<RecordingSegmentInformationListTrackFilter>();
  } else if (filterName == "tmva") {
    return makeUnique<TMVASegmentInformationListTrackFilter>();
  } else {
    return Super::create(filterName);
  }
}
