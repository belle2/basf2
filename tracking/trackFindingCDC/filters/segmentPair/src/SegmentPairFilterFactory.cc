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

#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>


using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::FilterFactory<BaseSegmentPairFilter>;

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
    return std::make_unique<NoneFilter<BaseSegmentPairFilter>>();
  } else if (filterName == "all") {
    return std::make_unique<AllSegmentPairFilter>();
  } else if (filterName == "truth") {
    return std::make_unique<MCSegmentPairFilter>();
  } else if (filterName == "fitless") {
    return std::make_unique<FitlessSegmentPairFilter>();
  } else if (filterName == "simple") {
    return std::make_unique<SimpleSegmentPairFilter>();
  } else if (filterName == "unionrecording") {
    return std::make_unique<UnionRecordingSegmentPairFilter>();
  } else if (filterName == "feasible") {
    return std::make_unique<MVAFeasibleSegmentPairFilter>();
  } else if (filterName == "realistic") {
    return std::make_unique<MVARealisticSegmentPairFilter>();
  } else {
    return Super::create(filterName);
  }
}
