/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/wireHit/WireHitFilterFactory.h>

#include <tracking/trackFindingCDC/filters/wireHit/BaseWireHitFilter.h>
#include <tracking/trackFindingCDC/filters/wireHit/AllWireHitFilter.h>
#include <tracking/trackFindingCDC/filters/wireHit/CDCWireHitVarSet.h>
#include <tracking/trackFindingCDC/filters/wireHit/CutsFromDBWireHitFilter.h>
#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>
#include <tracking/trackFindingCDC/filters/base/MVAFilter.icc.h>


using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// MVA filter for CDC states
  using MVACDCWireHitFilter = MVAFilter<CDCWireHitVarSet>;
}

template class TrackFindingCDC::FilterFactory<BaseWireHitFilter>;

WireHitFilterFactory::WireHitFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

WireHitFilterFactory::~WireHitFilterFactory() = default;

std::string WireHitFilterFactory::getIdentifier() const
{
  return "WireHit";
}

std::string WireHitFilterFactory::getFilterPurpose() const
{
  return "WireHit filter to reject background. ";
}

std::map<std::string, std::string> WireHitFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"all", "all wireHits are valid."},
    {"cuts_from_DB", "wireHits filtered by cuts from DB."},
    {"mva", "MVA filter using TOT, ADC, TDC to tag background hits."}
  };
}

std::unique_ptr<Filter<CDCWireHit> >
WireHitFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "all") {
    return std::make_unique<AllWireHitFilter>();
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "cuts_from_DB") {
    return std::make_unique<CutsFromDBWireHitFilter>();
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "mva") {
    return std::make_unique<MVACDCWireHitFilter>();
  } else {
    return Super::create(filterName);
  }
}
