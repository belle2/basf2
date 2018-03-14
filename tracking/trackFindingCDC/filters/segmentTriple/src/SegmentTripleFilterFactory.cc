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

#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>


using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::FilterFactory<BaseSegmentTripleFilter>;

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
    return std::make_unique<NoneFilter<BaseSegmentTripleFilter>>();
  } else if (filterName == "all") {
    return std::make_unique<AllSegmentTripleFilter>();
  } else if (filterName == "truth") {
    return std::make_unique<MCSegmentTripleFilter>();
  } else if (filterName == "simple") {
    return std::make_unique<SimpleSegmentTripleFilter>();
  } else {
    return Super::create(filterName);
  }
}
