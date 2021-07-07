/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/vtx/filters/results/VTXResultFilterFactory.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/ChoosableFromVarSetFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/MVAFilter.icc.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

#include <tracking/ckf/vtx/filters/results/VTXResultVarSet.h>
#include <tracking/ckf/vtx/filters/results/RelationVTXResultVarSet.h>
#include <tracking/ckf/vtx/filters/results/VTXResultTruthVarSet.h>
#include <tracking/ckf/vtx/filters/results/SizeVTXResultFilter.h>
#include <tracking/ckf/vtx/filters/results/WeightVTXResultFilter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// MC filter using the truth/teacher information
  using ChooseableTruthVTXResultFilter = ChoosableFromVarSetFilter<VTXResultTruthVarSet>;

  /// Basic recording filter for VTX - CDC results.
  using RecordingVTXResultFilter = RecordingFilter<VariadicUnionVarSet<VTXResultTruthVarSet, VTXResultVarSet>>;

  /// Basic recording filter for VTX - CDC results.
  using RecordingVTXSeededResultFilter =
    RecordingFilter<VariadicUnionVarSet<VTXResultTruthVarSet, VTXResultVarSet, RelationVTXResultVarSet>>;

  /// Filter using a trained MVA method
  using MVAVTXResultFilter = MVAFilter<VTXResultVarSet>;

  /// Filter using a trained MVA method
  using MVAVTXSeededResultFilter = MVAFilter<VariadicUnionVarSet<VTXResultVarSet, RelationVTXResultVarSet>>;
}


VTXResultFilterFactory::VTXResultFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

VTXResultFilterFactory::~VTXResultFilterFactory() = default;

std::string VTXResultFilterFactory::getIdentifier() const
{
  return "VTXResult";
}

std::string VTXResultFilterFactory::getFilterPurpose() const
{
  return "Reject VTX results. ";
}

std::map<std::string, std::string> VTXResultFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no combination is valid"},
    {"all", "all combination are valid"},
    {"recording", "record variables to a TTree"},
    {"recording_with_relations", "record variables to a TTree"},
    {"mva", "filter based on the trained MVA method"},
    {"mva_with_relations", "filter based on the trained MVA method"},
    {"size", "ordering accoring to size"},
    {"weight", "ordering according to weight"},
    {"truth", "monte carlo truth"},
    {"truth_teacher", "monte carlo truth returning the result of the teacher"},
    {"truth_vtx_cdc_relation", "monte carlo truth on the related CDC and VTX tracks"},
  };
}

std::unique_ptr<BaseVTXResultFilter>
VTXResultFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return std::make_unique<TrackFindingCDC::NoneFilter<BaseVTXResultFilter>>();
  } else if (filterName == "all") {
    return std::make_unique<TrackFindingCDC::AllFilter<BaseVTXResultFilter>>();
  } else if (filterName == "recording") {
    return std::make_unique<RecordingVTXResultFilter>();
  } else if (filterName == "recording_with_relations") {
    return std::make_unique<RecordingVTXSeededResultFilter>();
  } else if (filterName == "mva") {
    return std::make_unique<MVAVTXResultFilter>("ckf_CDCToVTXResult");
  } else if (filterName == "mva_with_relations") {
    return std::make_unique<MVAVTXSeededResultFilter>("ckf_SeededCDCToVTXResult");
  } else if (filterName == "truth") {
    return std::make_unique<ChooseableTruthVTXResultFilter>("truth");
  } else if (filterName == "truth_teacher") {
    return std::make_unique<ChooseableTruthVTXResultFilter>("truth_teacher");
  } else if (filterName == "truth_vtx_cdc_relation") {
    return std::make_unique<ChooseableTruthVTXResultFilter>("truth_vtx_cdc_relation");
  } else if (filterName == "size") {
    return std::make_unique<SizeVTXResultFilter>();
  } else if (filterName == "weight") {
    return std::make_unique<WeightVTXResultFilter>();
  } else {
    return Super::create(filterName);
  }
}
