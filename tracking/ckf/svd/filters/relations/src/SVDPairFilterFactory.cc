/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/svd/filters/relations/SVDPairFilterFactory.h>
#include <tracking/ckf/svd/filters/relations/SectorSVDPairFilter.h>
#include <tracking/ckf/svd/filters/relations/DistanceSVDPairFilter.h>
#include <tracking/ckf/svd/filters/relations/SectorMapBasedSVDPairFilter.h>

#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>
#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;


SVDPairFilterFactory::SVDPairFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

SVDPairFilterFactory::~SVDPairFilterFactory() = default;

std::string SVDPairFilterFactory::getIdentifier() const
{
  return "SVDPair";
}

std::string SVDPairFilterFactory::getFilterPurpose() const
{
  return "Reject svd pairs. ";
}

std::map<std::string, std::string> SVDPairFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"all", "all combinations are valid"},
    {"none", "no combination is valid"},
    {"sensor", "use sensor/ladder information"},
    {"sectormap", "use the sector map"},
    {"distance", "based on the position distance"},
  };
}

std::unique_ptr<BaseSVDPairFilter>
SVDPairFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "all") {
    return std::make_unique<TrackFindingCDC::AllFilter<BaseSVDPairFilter>>();
  }

  if (filterName == "none") {
    return std::make_unique<TrackFindingCDC::NoneFilter<BaseSVDPairFilter>>();
  }

  if (filterName == "sensor") {
    return std::make_unique<SectorSVDPairFilter>();
  }

  if (filterName == "distance") {
    return std::make_unique<DistanceSVDPairFilter>();
  }

  if (filterName == "sectormap") {
    return std::make_unique<SectorMapBasedSVDPairFilter>();
  }

  return Super::create(filterName);
}
