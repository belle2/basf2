/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/pxd/filters/relations/PXDPairFilterFactory.h>
#include <tracking/ckf/pxd/filters/relations/SensorPXDPairFilter.h>
#include <tracking/ckf/pxd/filters/relations/DistancePXDPairFilter.h>
#include <tracking/ckf/pxd/filters/relations/LoosePXDPairFilter.h>

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
  return "Reject PXD state pairs. ";
}

std::map<std::string, std::string> PXDPairFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"all", "all combinations are valid"},
    {"none", "no combination is valid"},
    {"sensor", "use sensor/ladder information"},
    {"distance", "based on the position distance"},
    {"loose", "loose prefilter"},
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
    return std::make_unique<SensorPXDPairFilter>();
  }

  if (filterName == "distance") {
    return std::make_unique<DistancePXDPairFilter>();
  }

  if (filterName == "loose") {
    return std::make_unique<LoosePXDPairFilter>();
  }

  return Super::create(filterName);
}
