/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/vtx/filters/relations/VTXPairFilterFactory.h>
#include <tracking/ckf/vtx/filters/relations/SensorVTXPairFilter.h>
#include <tracking/ckf/vtx/filters/relations/DistanceVTXPairFilter.h>
#include <tracking/ckf/vtx/filters/relations/SectorMapBasedVTXPairFilter.h>
#include <tracking/ckf/vtx/filters/relations/LooseVTXPairFilter.h>

#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>
#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;


VTXPairFilterFactory::VTXPairFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

VTXPairFilterFactory::~VTXPairFilterFactory() = default;

std::string VTXPairFilterFactory::getIdentifier() const
{
  return "VTXPair";
}

std::string VTXPairFilterFactory::getFilterPurpose() const
{
  return "Reject VTX pairs. ";
}

std::map<std::string, std::string> VTXPairFilterFactory::getValidFilterNamesAndDescriptions() const
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

std::unique_ptr<BaseVTXPairFilter>
VTXPairFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "all") {
    return std::make_unique<TrackFindingCDC::AllFilter<BaseVTXPairFilter>>();
  }

  if (filterName == "none") {
    return std::make_unique<TrackFindingCDC::NoneFilter<BaseVTXPairFilter>>();
  }

  if (filterName == "sensor") {
    return std::make_unique<SensorVTXPairFilter>();
  }

  if (filterName == "distance") {
    return std::make_unique<DistanceVTXPairFilter>();
  }

  if (filterName == "sectormap") {
    return std::make_unique<SectorMapBasedVTXPairFilter>();
  }

  if (filterName == "loose") {
    return std::make_unique<LooseVTXPairFilter>();
  }

  return Super::create(filterName);
}
