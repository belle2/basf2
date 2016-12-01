/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentTriple/SegmentTripleFilterFactory.h>

#include <tracking/trackFindingCDC/filters/segmentTriple/BaseSegmentTripleFilter.h>
#include <tracking/trackFindingCDC/filters/segmentTriple/AllSegmentTripleFilter.h>
#include <tracking/trackFindingCDC/filters/segmentTriple/MCSegmentTripleFilter.h>
#include <tracking/trackFindingCDC/filters/segmentTriple/SimpleSegmentTripleFilter.h>

#include <tracking/trackFindingCDC/utilities/MakeUnique.h>

using namespace Belle2;
using namespace TrackFindingCDC;

SegmentTripleFilterFactory::SegmentTripleFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string SegmentTripleFilterFactory::getIdentifier() const
{
  return "SegmentTriple";
}

std::string SegmentTripleFilterFactory::getFilterPurpose() const
{
  return "Segment triple filter to construct of a segment triple network";
}

std::map<std::string, std::string>
SegmentTripleFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no segment triple is valid"},
    {"all", "all segment triples are valid"},
    {"truth", "monte carlo truth"},
    {"simple", "mc free with simple criteria"},
  };
}

std::unique_ptr<Filter<CDCSegmentTriple> >
SegmentTripleFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return makeUnique<BaseSegmentTripleFilter>();
  } else if (filterName == "all") {
    return makeUnique<AllSegmentTripleFilter>();
  } else if (filterName == "truth") {
    return makeUnique<MCSegmentTripleFilter>();
  } else if (filterName == "simple") {
    return makeUnique<SimpleSegmentTripleFilter>();
  } else {
    return Super::create(filterName);
  }
}
