/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPair/SegmentPairFilterFactory.h>

#include <tracking/trackFindingCDC/filters/segmentPair/BaseSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPair/AllSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPair/MCSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPair/FitlessSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPair/SimpleSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPair/UnionRecordingSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPair/MVAFeasibleSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPair/MVARealisticSegmentPairFilter.h>

#include <tracking/trackFindingCDC/utilities/MakeUnique.h>

using namespace Belle2;
using namespace TrackFindingCDC;

SegmentPairFilterFactory::SegmentPairFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string SegmentPairFilterFactory::getIdentifier() const
{
  return "SegmentPair";
}

std::string SegmentPairFilterFactory::getFilterPurpose() const
{
  return "Segment pair filter to construct of a segment pair network";
}

std::map<std::string, std::string>
SegmentPairFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no segment pair is valid"},
    {"all", "all segment pairs are valid"},
    {"truth", "monte carlo truth"},
    {"fitless", "mc free with simple criteria without the common fit"},
    {"simple", "mc free with simple criteria"},
    {"unionrecording", "record many multiple choosable variable set"},
    {"feasible", "multivariat method based on variables of the first and last hit in each segment meant as precut"},
    {"realistic", "realistic filter using a common fit and combination of all information with an mva"},
  };
}

std::unique_ptr<Filter<CDCSegmentPair> >
SegmentPairFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return makeUnique<BaseSegmentPairFilter>();
  } else if (filterName == "all") {
    return makeUnique<AllSegmentPairFilter>();
  } else if (filterName == "truth") {
    return makeUnique<MCSegmentPairFilter>();
  } else if (filterName == "fitless") {
    return makeUnique<FitlessSegmentPairFilter>();
  } else if (filterName == "simple") {
    return makeUnique<SimpleSegmentPairFilter>();
  } else if (filterName == "unionrecording") {
    return makeUnique<UnionRecordingSegmentPairFilter>();
  } else if (filterName == "feasible") {
    return makeUnique<MVAFeasibleSegmentPairFilter>();
  } else if (filterName == "realistic") {
    return makeUnique<MVARealisticSegmentPairFilter>();
  } else {
    return Super::create(filterName);
  }
}
