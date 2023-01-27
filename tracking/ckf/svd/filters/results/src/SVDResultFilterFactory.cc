/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/svd/filters/results/SVDResultFilterFactory.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/ChoosableFromVarSetFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/MVAFilter.icc.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

#include <tracking/ckf/svd/filters/results/SVDResultVarSet.h>
#include <tracking/ckf/svd/filters/results/RelationSVDResultVarSet.h>
#include <tracking/ckf/svd/filters/results/SVDResultTruthVarSet.h>
#include <tracking/ckf/svd/filters/results/SizeSVDResultFilter.h>
#include <tracking/ckf/svd/filters/results/WeightSVDResultFilter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// MC filter using the truth/teacher information
  using ChooseableTruthSVDResultFilter = ChoosableFromVarSetFilter<SVDResultTruthVarSet>;

  /// Basic recording filter for SVD - CDC results.
  using RecordingSVDResultFilter =
    RecordingFilter<VariadicUnionVarSet<SVDResultTruthVarSet, SVDResultVarSet>>;

  /// Basic recording filter for SVD - CDC results.
  using RecordingSVDSeededResultFilter =
    RecordingFilter<VariadicUnionVarSet<SVDResultTruthVarSet, SVDResultVarSet, RelationSVDResultVarSet>>;

  /// Filter using a trained MVA method
  using MVASVDResultFilter = MVAFilter<SVDResultVarSet>;

  /// Filter using a trained MVA method
  using MVASVDSeededResultFilter = MVAFilter<VariadicUnionVarSet<SVDResultVarSet, RelationSVDResultVarSet>>;
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
  return "Reject SVD CKF results. ";
}

std::map<std::string, std::string> SVDResultFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no combination is valid"},
    {"all", "all combination are valid"},
    {"recording", "record variables to a TTree"},
    {"recording_with_relations", "record variables to a TTree"},
    {"mva", "filter based on the trained MVA method"},
    {"mva_with_relations", "filter based on the trained MVA method"},
    {"size", "ordering according to size"},
    {"weight", "ordering according to weight"},
    {"truth", "monte carlo truth"},
    {"truth_svd_cdc_relation", "monte carlo truth on the related CDC and SVD tracks"},
  };
}

std::unique_ptr<BaseSVDResultFilter>
SVDResultFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return std::make_unique<TrackFindingCDC::NoneFilter<BaseSVDResultFilter>>();
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "all") {
    return std::make_unique<TrackFindingCDC::AllFilter<BaseSVDResultFilter>>();
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "recording") {
    return std::make_unique<RecordingSVDResultFilter>();
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "recording_with_relations") {
    return std::make_unique<RecordingSVDSeededResultFilter>();
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "mva") {
    return std::make_unique<MVASVDResultFilter>("ckf_CDCToSVDResult");
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "mva_with_relations") {
    return std::make_unique<MVASVDSeededResultFilter>("ckf_SeededCDCToSVDResult");
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "truth") {
    return std::make_unique<ChooseableTruthSVDResultFilter>("truth");
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "truth_svd_cdc_relation") {
    return std::make_unique<ChooseableTruthSVDResultFilter>("truth_svd_cdc_relation");
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "size") {
    return std::make_unique<SizeSVDResultFilter>();
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "weight") {
    return std::make_unique<WeightSVDResultFilter>();
  } else {
    return Super::create(filterName);
  }
}
