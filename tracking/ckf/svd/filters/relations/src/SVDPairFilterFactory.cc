/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/svd/filters/relations/SVDPairFilterFactory.h>
#include <tracking/ckf/svd/filters/relations/SensorSVDPairFilter.h>
#include <tracking/ckf/svd/filters/relations/DistanceSVDPairFilter.h>
#include <tracking/ckf/svd/filters/relations/SectorMapBasedSVDPairFilter.h>
#include <tracking/ckf/svd/filters/relations/LooseSVDPairFilter.h>

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
  return "Reject SVD state pairs. ";
}

std::map<std::string, std::string> SVDPairFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"all", "all combinations are valid"},
    {"none", "no combination is valid"},
    {"sensor", "use sensor/ladder information"},
    {"sectormap", "use the sector map"},
    {"distance", "based on the position distance"},
    {"loose", "loose prefilter"},
  };
}

std::unique_ptr<BaseSVDPairFilter>
SVDPairFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "all") {
    return std::make_unique<TrackFindingCDC::AllFilter<BaseSVDPairFilter>>();
  }

  // cppcheck-suppress knownConditionTrueFalse
  if (filterName == "none") {
    return std::make_unique<TrackFindingCDC::NoneFilter<BaseSVDPairFilter>>();
  }

  // cppcheck-suppress knownConditionTrueFalse
  if (filterName == "sensor") {
    return std::make_unique<SensorSVDPairFilter>();
  }

  if (filterName == "distance") {
    return std::make_unique<DistanceSVDPairFilter>();
  }

  if (filterName == "sectormap") {
    return std::make_unique<SectorMapBasedSVDPairFilter>();
  }

  if (filterName == "loose") {
    return std::make_unique<LooseSVDPairFilter>();
  }

  return Super::create(filterName);
}
