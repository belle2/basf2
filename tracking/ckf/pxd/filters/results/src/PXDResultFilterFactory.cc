/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/pxd/filters/results/PXDResultFilterFactory.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/ChoosableFromVarSetFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/MVAFilter.icc.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

#include <tracking/ckf/pxd/filters/results/PXDResultVarSet.h>
#include <tracking/ckf/pxd/filters/results/PXDResultTruthVarSet.h>
#include <tracking/ckf/pxd/filters/results/SizePXDResultFilter.h>


using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// MC filter using the truth/teacher information
  using ChooseableTruthPXDResultFilter = ChoosableFromVarSetFilter<PXDResultTruthVarSet>;

  /// Basic recording filter for PXD - CDC results.
  using RecordingPXDResultFilter = RecordingFilter<VariadicUnionVarSet<PXDResultTruthVarSet, PXDResultVarSet>>;

  /// Filter using a trained MVA method
  using MVAPXDResultFilter = MVAFilter<PXDResultVarSet>;
}


PXDResultFilterFactory::PXDResultFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

PXDResultFilterFactory::~PXDResultFilterFactory() = default;

std::string PXDResultFilterFactory::getIdentifier() const
{
  return "PXDResult";
}

std::string PXDResultFilterFactory::getFilterPurpose() const
{
  return "Reject PXD CKF results. ";
}

std::map<std::string, std::string> PXDResultFilterFactory::getValidFilterNamesAndDescriptions() const
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

std::unique_ptr<BasePXDResultFilter>
PXDResultFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return std::make_unique<TrackFindingCDC::NoneFilter<BasePXDResultFilter>>();
  } else if (filterName == "all") {
    return std::make_unique<TrackFindingCDC::AllFilter<BasePXDResultFilter>>();
  } else if (filterName == "recording") {
    return std::make_unique<RecordingPXDResultFilter>();
  } else if (filterName == "mva") {
    return std::make_unique<MVAPXDResultFilter>("ckf_CDCToPXDResult");
  } else if (filterName == "truth") {
    return std::make_unique<ChooseableTruthPXDResultFilter>("truth");
  } else if (filterName == "truth_teacher") {
    return std::make_unique<ChooseableTruthPXDResultFilter>("truth_teacher");
  } else if (filterName == "size") {
    return std::make_unique<SizePXDResultFilter>();
  } else {
    return Super::create(filterName);
  }
}
