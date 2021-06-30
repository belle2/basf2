/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/vxdHoughTracking/filters/relations/RelationFilterFactory.h>
#include <tracking/vxdHoughTracking/filters/relations/AngleAndTimeRelationFilter.h>
#include <tracking/vxdHoughTracking/filters/relations/SimpleRelationFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>
#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace vxdHoughTracking;

RelationFilterFactory::RelationFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

RelationFilterFactory::~RelationFilterFactory() = default;

std::string RelationFilterFactory::getIdentifier() const
{
  return "Factory for SVD Hit Pair Relation Filters";
}

std::string RelationFilterFactory::getFilterPurpose() const
{
  return "Rejects SVD hit pairs based on their relation. ";
}

std::map<std::string, std::string> RelationFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"all", "all combinations are valid"},
    {"none", "no combination is valid"},
    {"angleAndTime", "filter relation based on their theta value and the hit time"},
    {"simple", "very simple filtering based on theta of the hits in question"},
  };
}

std::unique_ptr<BaseRelationFilter>
RelationFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "all") {
    return std::make_unique<TrackFindingCDC::AllFilter<BaseRelationFilter>>();
  }

  if (filterName == "none") {
    return std::make_unique<TrackFindingCDC::NoneFilter<BaseRelationFilter>>();
  }

  if (filterName == "angleAndTime") {
    return std::make_unique<AngleAndTimeRelationFilter>();
  }

  if (filterName == "simple") {
    return std::make_unique<SimpleRelationFilter>();
  }

  return Super::create(filterName);
}
