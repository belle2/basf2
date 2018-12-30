/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/cdc/filters/paths/CDCPathFilterFactory.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>

#include <tracking/ckf/cdc/filters/paths/SizeCDCPathFilter.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/MVAFilter.icc.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

using namespace Belle2;
using namespace TrackFindingCDC;


CDCPathFilterFactory::CDCPathFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

CDCPathFilterFactory::~CDCPathFilterFactory() = default;

std::string CDCPathFilterFactory::getIdentifier() const
{
  return "CDCCKFPath";
}

std::string CDCPathFilterFactory::getFilterPurpose() const
{
  return "Reject cdc ckf paths";
}

std::map<std::string, std::string> CDCPathFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no track combination is valid"},
    {"size", "very rough filtering"},
    //{"mc_truth", "Extrapolation and update"},
  };
}

std::unique_ptr<BaseCDCPathFilter>
CDCPathFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return std::make_unique<TrackFindingCDC::NoneFilter<BaseCDCPathFilter>>();
  } else if (filterName == "all") {
    return std::make_unique<TrackFindingCDC::AllFilter<BaseCDCPathFilter>>();
  } else if (filterName == "size") {
    return std::make_unique<SizeCDCPathFilter>();
    //} else if (filterName == "mc_truth") {
    //  return std::make_unique<MCTruthCDCPathFilter>();
  } else {
    return Super::create(filterName);
  }
}
