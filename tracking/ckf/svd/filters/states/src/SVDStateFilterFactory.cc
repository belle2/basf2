/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/svd/filters/states/SVDStateFilterFactory.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/ChoosableFromVarSetFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/MVAFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/SloppyFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/TruthVarFilter.icc.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

#include <tracking/ckf/svd/filters/states/SVDStateVarSet.h>
#include <tracking/ckf/svd/filters/states/SVDStateBasicVarSet.h>
#include <tracking/ckf/svd/filters/states/SVDStateTruthVarSet.h>
#include <tracking/ckf/svd/filters/states/SimpleSVDStateFilter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// MC filter for VXD - CDC relations.
  using MCSVDStateFilter = TruthVarFilter<SVDStateTruthVarSet>;

  /// MC-ordering filter for VXD - CDC relations.
  using MCOrderingSVDStateFilter = ChoosableFromVarSetFilter<SVDStateTruthVarSet>;

  /// Sloppy MC filter for VXD - CDC relations.
  using SloppyMCSVDStateFilter = Sloppy<TruthVarFilter<SVDStateTruthVarSet>>;

  /// Recording filter for VXD - CDC relations.
  using RecordingSVDStateFilter = RecordingFilter<VariadicUnionVarSet<SVDStateTruthVarSet, SVDStateBasicVarSet, SVDStateVarSet>>;

  /// Prescaled recording filter for VXD - CDC relations.
  class SloppyRecordingSVDStateFilter : public RecordingSVDStateFilter {
  public:
    SloppyRecordingSVDStateFilter(const std::string& defaultRootFileName) : RecordingSVDStateFilter(defaultRootFileName)
    {
      setSkimFilter(std::make_unique<SloppyMCSVDStateFilter>());
    }
  };
}


SVDStateFilterFactory::SVDStateFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

SVDStateFilterFactory::~SVDStateFilterFactory() = default;

std::string SVDStateFilterFactory::getIdentifier() const
{
  return "SVDState";
}

std::string SVDStateFilterFactory::getFilterPurpose() const
{
  return "Reject svd states";
}

std::map<std::string, std::string> SVDStateFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no track combination is valid"},
    {"all", "set all track combinations as good"},
    {"truth", "monte carlo truth"},
    {"ordering_truth", "monte carlo truth ordering"},
    {"sloppy_truth", "sloppy monte carlo truth"},
    {"simple", "simple filter to be used in svd"},
    {"recording", "record variables to a TTree"},
    {"mva", "MVA filter"},
    {"sloppy_recording", "record variables to a TTree"},
  };
}

std::unique_ptr<BaseSVDStateFilter>
SVDStateFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return std::make_unique<TrackFindingCDC::NoneFilter<BaseSVDStateFilter>>();
  } else if (filterName == "all") {
    return std::make_unique<TrackFindingCDC::AllFilter<BaseSVDStateFilter>>();
  } else if (filterName == "simple") {
    return std::make_unique<SimpleSVDStateFilter>();
  } else if (filterName == "truth") {
    return std::make_unique<MCSVDStateFilter>();
  } else if (filterName == "ordering_truth") {
    return std::make_unique<MCOrderingSVDStateFilter>("truth_inverted");
  } else if (filterName == "sloppy_truth") {
    return std::make_unique<SloppyMCSVDStateFilter>();
  } else if (filterName == "recording") {
    return std::make_unique<RecordingSVDStateFilter>("SVDStateFilter.root");
    // TODO} else if (filterName == "mva") {
    //  return std::make_unique<MVASVDStateFilter>("tracking/data/ckf_CDCSVDStateFilter_1.xml");
  } else if (filterName == "sloppy_recording") {
    return std::make_unique<SloppyRecordingSVDStateFilter>("SVDStateFilter.root");
  } else {
    return Super::create(filterName);
  }
}
