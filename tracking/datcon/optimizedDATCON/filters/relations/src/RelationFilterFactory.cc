/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/optimizedDATCON/filters/relations/RelationFilterFactory.h>
#include <tracking/datcon/optimizedDATCON/filters/relations/SimpleRelationFilter.h>

#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>
#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;


RelationFilterFactory::RelationFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

RelationFilterFactory::~RelationFilterFactory() = default;

std::string RelationFilterFactory::getIdentifier() const
{
  return "SVD Hit Pair";
}

std::string RelationFilterFactory::getFilterPurpose() const
{
  return "Rejects SVD hit pairs. ";
}

std::map<std::string, std::string> RelationFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"all", "all combinations are valid"},
    {"none", "no combination is valid"},
    {"simple", "very simple filtering"},
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

  if (filterName == "simple") {
    return std::make_unique<SimpleRelationFilter>();
  }

  return Super::create(filterName);
}
