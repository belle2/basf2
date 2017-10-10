/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/svd/filters/relations/SVDRelationFilterFactory.h>
#include <tracking/ckf/svd/filters/relations/SectorSVDRelationFilter.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;


SVDRelationFilterFactory::SVDRelationFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

SVDRelationFilterFactory::~SVDRelationFilterFactory() = default;

std::string SVDRelationFilterFactory::getIdentifier() const
{
  return "SVDRelation";
}

std::string SVDRelationFilterFactory::getFilterPurpose() const
{
  return "Reject svd relations. ";
}

std::map<std::string, std::string> SVDRelationFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"all", "all combinations are valid"},
    {"sensor", "use sensor/ladder information"},
  };
}

std::unique_ptr<LayerSVDRelationFilter>
SVDRelationFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "all") {
    return std::make_unique<LayerSVDRelationFilter>();
  } else if (filterName == "sensor") {
    return std::make_unique<SectorSVDRelationFilter>();
  } else {
    return Super::create(filterName);
  }
}
