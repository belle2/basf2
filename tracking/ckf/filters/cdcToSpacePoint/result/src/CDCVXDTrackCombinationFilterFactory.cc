/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/filters/cdcToSpacePoint/result/CDCVXDTrackCombinationFilterFactory.h>
#include <tracking/ckf/filters/cdcToSpacePoint/result/BaseCDCVXDTrackCombinationFilter.h>
#include <tracking/ckf/filters/cdcToSpacePoint/result/SimpleCDCVXDTrackCombinationFilter.h>
#include <tracking/ckf/filters/cdcToSpacePoint/result/Chi2CDCVXDTrackCombinationFilter.h>
#include <tracking/ckf/filters/cdcToSpacePoint/result/CDCVXDTrackCombinationTruthVarSet.h>
#include <tracking/ckf/filters/cdcToSpacePoint/result/CDCVXDTrackCombinationVarSet.h>

#include <tracking/trackFindingCDC/filters/base/MCFilter.h>
#include <tracking/trackFindingCDC/filters/base/MVAFilter.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>
#include <tracking/trackFindingCDC/filters/base/NamedChoosableVarSetFilter.h>
#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// MC filter for VXD - CDC relations.
  using MCCDCVXDTrackCombinationFilter = MCFilter<CDCVXDTrackCombinationTruthVarSet>;

  /// MC filter for VXD - CDC relations.
  using MCCTruthNumberCDCVXDTrackCombinationFilter = NamedChoosableVarSetFilter<CDCVXDTrackCombinationTruthVarSet>;

  /// Basic recording filter for VXD - CDC relations.
  using RecordingCDCVXDTrackCombinationFilter =
    RecordingFilter<VariadicUnionVarSet<CDCVXDTrackCombinationTruthVarSet, CDCVXDTrackCombinationVarSet>>;

  /// All filter for VXD - CDC relations.
  using AllCDCVXDTrackCombinationFilter = AllFilter<BaseCDCVXDTrackCombinationFilter>;

  /// Filter using a trained MVA method
  using MVACDCVXDTrackCombinationFilter = MVAFilter<CDCVXDTrackCombinationVarSet>;

  /// Filter using a trained MVA method for skimming, but returns the number of hits
  class MVASkimmedCDCVXDTrackCombinationFilter : public MVACDCVXDTrackCombinationFilter {
  private:
    using Super = MVACDCVXDTrackCombinationFilter;

  public:
    using Super::Super;

    /// Initialisation method sets up a reference to the value in the variable set to be returned.
    void initialize() override
    {
      Super::initialize();
      MayBePtr<Float_t> foundVariable = Super::getVarSet().find(m_param_varName);
      if (not foundVariable) {
        B2ERROR("Could not find request variable name " << m_param_varName << " in variable set");
      }
      m_variable = foundVariable;
    }

    Weight operator()(const MVACDCVXDTrackCombinationFilter::Object& obj) override
    {
      Weight prediction = Super::operator()(obj);
      if (std::isnan(prediction)) {
        return NAN;
      } else {
        return *m_variable;
      }
    }

  private:
    /// Memory for the name of the variable selected as the return value of the filter.
    std::string m_param_varName = "number_of_hits";

    /// Reference to the location of the value in the variable set to be returned
    Float_t* m_variable = nullptr;
  };
}

CDCVXDTrackCombinationFilterFactory::CDCVXDTrackCombinationFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string CDCVXDTrackCombinationFilterFactory::getIdentifier() const
{
  return "CDCVXDTrackCombination";
}

std::string CDCVXDTrackCombinationFilterFactory::getFilterPurpose() const
{
  return "Filter out relations between CDC Reco Tracks and Space Points on very easy criteria";
}

std::map<std::string, std::string>
CDCVXDTrackCombinationFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no track combination is valid"},
    {"all", "set all track combinations as good"},
    {"recording", "record variables to a TTree"},
    {"mva", "filter based on the trained MVA method"},
    {"mva_skimmed", "filter based on the trained MVA method for skimming, bur returns the number of hits"},
    {"truth", "monte carlo truth"},
    {"truth_number", "monte carlo truth returning the number of correct hits"},
    {"truth_teacher", "monte carlo truth returning the result of the teacher"},
    {"simple", "simple filter based on simple parameters"},
    {"chi2", "filter based on a chi2 calculation"},
  };
}

std::unique_ptr<BaseCDCVXDTrackCombinationFilter>
CDCVXDTrackCombinationFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return makeUnique<NoneFilter<BaseCDCVXDTrackCombinationFilter>>();
  } else if (filterName == "all") {
    return makeUnique<AllCDCVXDTrackCombinationFilter >();
  } else if (filterName == "recording") {
    return makeUnique<RecordingCDCVXDTrackCombinationFilter>();
  } else if (filterName == "mva") {
    return makeUnique<MVACDCVXDTrackCombinationFilter>("tracking/data/ckf_CDCVXDTrackCombination.xml");
  } else if (filterName == "mva_skimmed") {
    return makeUnique<MVASkimmedCDCVXDTrackCombinationFilter>("tracking/data/ckf_CDCVXDTrackCombination.xml");
  } else if (filterName == "truth") {
    return makeUnique<MCCDCVXDTrackCombinationFilter >();
  } else if (filterName == "truth_number") {
    return makeUnique<MCCTruthNumberCDCVXDTrackCombinationFilter>("truth_number_of_correct_hits");
  } else if (filterName == "truth_teacher") {
    return makeUnique<MCCTruthNumberCDCVXDTrackCombinationFilter>("truth_teacher");
  } else if (filterName == "simple") {
    return makeUnique<SimpleCDCVXDTrackCombinationFilter>();
  } else if (filterName == "chi2") {
    return makeUnique<Chi2CDCVXDTrackCombinationFilter>();
  } else {
    return Super::create(filterName);
  }
}
