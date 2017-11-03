/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/svd/filters/results/SVDResultFilterFactory.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/ChoosableFromVarSetFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/MVAFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/TruthVarFilter.icc.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

#include <tracking/ckf/svd/filters/results/SVDResultVarSet.h>
#include <tracking/ckf/svd/filters/results/SVDResultTruthVarSet.h>
#include <tracking/ckf/svd/filters/results/SizeSVDResultFilter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// MC filter using the truth/teacher information
  using ChooseableTruthSVDResultFilter = ChoosableFromVarSetFilter<SVDResultTruthVarSet>;

  /// Basic recording filter for SVD - CDC results.
  using RecordingSVDResultFilter = RecordingFilter<VariadicUnionVarSet<SVDResultTruthVarSet, SVDResultVarSet>>;

  /// Filter using a trained MVA method
  using MVASVDResultFilter = MVAFilter<SVDResultVarSet>;
}


SVDResultFilterFactory::SVDResultFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

SVDResultFilterFactory::~SVDResultFilterFactory() = default;

std::string SVDResultFilterFactory::getIdentifier() const
{
  return "SVDResult";
}

std::string SVDResultFilterFactory::getFilterPurpose() const
{
  return "Reject svd results";
}

std::map<std::string, std::string> SVDResultFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no combination is valid"},
    {"all", "all combination are valid"},
    {"recording", "record variables to a TTree"},
    {"mva", "filter based on the trained MVA method"},
    {"size", "ordering accoring to size"},
    {"truth", "monte carlo truth"},
    {"truth_teacher", "monte carlo truth returning the result of the teacher"},
  };
}

std::unique_ptr<BaseSVDResultFilter>
SVDResultFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return std::make_unique<TrackFindingCDC::NoneFilter<BaseSVDResultFilter>>();
  } else if (filterName == "all") {
    return std::make_unique<TrackFindingCDC::AllFilter<BaseSVDResultFilter>>();
  } else if (filterName == "recording") {
    return std::make_unique<RecordingSVDResultFilter>();
  } else if (filterName == "mva") {
    return std::make_unique<MVASVDResultFilter>("tracking/data/ckf_CDCToSVDResult.xml");
  } else if (filterName == "truth") {
    return std::make_unique<ChooseableTruthSVDResultFilter>("truth");
  } else if (filterName == "truth_teacher") {
    return std::make_unique<ChooseableTruthSVDResultFilter>("truth_teacher");
  } else if (filterName == "size") {
    return std::make_unique<SizeSVDResultFilter>();
  } else {
    return Super::create(filterName);
  }
}
