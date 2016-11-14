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

using namespace Belle2;
using namespace TrackFindingCDC;

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
    {"all", "set all segments as good"},
    {"truth", "monte carlo truth"},
    {"recording", "record variables to a TTree"},
    {"tmva", "test with a tmva method"}
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
  } else if (filterName == "tmva") {
    return makeUnique<TMVABackgroundSegmentFilter>("BackgroundSegmentFilter");
  } else {
    return Super::create(filterName);
  }
}
