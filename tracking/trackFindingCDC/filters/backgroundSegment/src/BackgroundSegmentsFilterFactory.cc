/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/backgroundSegment/BackgroundSegmentsFilterFactory.h>

using namespace Belle2;
using namespace TrackFindingCDC;

BackgroundSegmentsFilterFactory::BackgroundSegmentsFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string BackgroundSegmentsFilterFactory::getIdentifier() const
{
  return "BackgroundSegments";
}

std::string BackgroundSegmentsFilterFactory::getFilterPurpose() const
{
  return "Segment filter to reject background";
}

std::map<std::string, std::string>
BackgroundSegmentsFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no segment track combination is valid"},
    {"all", "set all segments as good"},
    {"truth", "monte carlo truth"},
    {"recording", "record variables to a TTree"},
    {"tmva", "test with a tmva method"}
  };
}

std::unique_ptr<BaseBackgroundSegmentsFilter>
BackgroundSegmentsFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return makeUnique<BaseBackgroundSegmentsFilter>();
  } else if (filterName == "all") {
    return makeUnique<AllBackgroundSegmentsFilter>();
  } else if (filterName == "truth") {
    return makeUnique<MCBackgroundSegmentsFilter>();
  } else if (filterName == "recording") {
    return makeUnique<RecordingBackgroundSegmentsFilter>("BackgroundSegmentsFilter.root");
  } else if (filterName == "tmva") {
    return makeUnique<TMVABackgroundSegmentsFilter>("BackgroundSegmentsFilter");
  } else {
    return Super::create(filterName);
  }
}
