/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/axialSegmentPair/AxialSegmentPairFilterFactory.h>

#include <tracking/trackFindingCDC/filters/axialSegmentPair/BaseAxialSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/axialSegmentPair/AllAxialSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/axialSegmentPair/MCAxialSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/axialSegmentPair/SimpleAxialSegmentPairFilter.h>

#include <tracking/trackFindingCDC/filters/base/NoneFilter.h>


using namespace Belle2;
using namespace TrackFindingCDC;

AxialSegmentPairFilterFactory::AxialSegmentPairFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string AxialSegmentPairFilterFactory::getIdentifier() const
{
  return "AxialSegmentPair";
}

std::string AxialSegmentPairFilterFactory::getFilterPurpose() const
{
  return "AxialSegment pair filter to construct of a axialSegment pair network";
}

std::map<std::string, std::string>
AxialSegmentPairFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no axialSegment pair is valid"},
    {"all", "all axialSegment pairs are valid"},
    {"truth", "monte carlo truth"},
    {"simple", "mc free with simple criteria"},
  };
}

std::unique_ptr<Filter<CDCAxialSegmentPair> >
AxialSegmentPairFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return std::make_unique<NoneFilter<BaseAxialSegmentPairFilter> >();
  } else if (filterName == "all") {
    return std::make_unique<AllAxialSegmentPairFilter>();
  } else if (filterName == "truth") {
    return std::make_unique<MCAxialSegmentPairFilter>();
  } else if (filterName == "simple") {
    return std::make_unique<SimpleAxialSegmentPairFilter>();
  } else {
    return Super::create(filterName);
  }
}
