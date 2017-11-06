/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/pxd/filters/relations/PXDPairFilterFactory.h>
#include <tracking/ckf/pxd/filters/relations/SectorPXDPairFilter.h>
#include <tracking/ckf/pxd/filters/relations/DistancePXDPairFilter.h>

#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>
#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;


PXDPairFilterFactory::PXDPairFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

PXDPairFilterFactory::~PXDPairFilterFactory() = default;

std::string PXDPairFilterFactory::getIdentifier() const
{
  return "PXDPair";
}

std::string PXDPairFilterFactory::getFilterPurpose() const
{
  return "Reject pxd pairs. ";
}

std::map<std::string, std::string> PXDPairFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"all", "all combinations are valid"},
    {"none", "no combination is valid"},
    {"sensor", "use sensor/ladder information"},
    {"distance", "based on the position distance"},
  };
}

std::unique_ptr<BasePXDPairFilter>
PXDPairFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "all") {
    return std::make_unique<TrackFindingCDC::AllFilter<BasePXDPairFilter>>();
  }

  if (filterName == "none") {
    return std::make_unique<TrackFindingCDC::NoneFilter<BasePXDPairFilter>>();
  }

  if (filterName == "sensor") {
    return std::make_unique<SectorPXDPairFilter>();
  }

  if (filterName == "distance") {
    return std::make_unique<DistancePXDPairFilter>();
  }

  return Super::create(filterName);
}
