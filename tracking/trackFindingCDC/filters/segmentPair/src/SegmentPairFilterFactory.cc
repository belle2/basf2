/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "all") {
    return std::make_unique<AllSegmentPairFilter>();
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "truth") {
    return std::make_unique<MCSegmentPairFilter>();
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "fitless") {
    return std::make_unique<FitlessSegmentPairFilter>();
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "simple") {
    return std::make_unique<SimpleSegmentPairFilter>();
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "unionrecording") {
    return std::make_unique<UnionRecordingSegmentPairFilter>();
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "feasible") {
    return std::make_unique<MVAFeasibleSegmentPairFilter>();
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "realistic") {
    return std::make_unique<MVARealisticSegmentPairFilter>();
  } else {
    return Super::create(filterName);
  }
}
