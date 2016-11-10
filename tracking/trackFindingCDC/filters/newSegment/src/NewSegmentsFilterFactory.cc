/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/newSegment/NewSegmentsFilterFactory.h>

using namespace Belle2;
using namespace TrackFindingCDC;

NewSegmentsFilterFactory::NewSegmentsFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}
std::string NewSegmentsFilterFactory::getIdentifier() const
{
  return "NewSegment";
}

std::string NewSegmentsFilterFactory::getFilterPurpose() const
{
  return "Segment background finder.";
}

std::map<std::string, std::string>
NewSegmentsFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no segment track combination is valid"},
    {"truth", "monte carlo truth"},
    {"recording", "record variables to a TTree"},
    {"tmva", "test with a tmva method"}
  };
}

std::unique_ptr<BaseNewSegmentsFilter>
NewSegmentsFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return makeUnique<BaseNewSegmentsFilter>();
  } else if (filterName == "truth") {
    return makeUnique<MCNewSegmentsFilter>();
  } else if (filterName == "recording") {
    return makeUnique<RecordingNewSegmentsFilter>("NewSegmentsFilter.root");
  } else if (filterName == "tmva") {
    return makeUnique<TMVANewSegmentsFilter>("NewSegmentsFilter");
  } else {
    return Super::create(filterName);
  }
}
