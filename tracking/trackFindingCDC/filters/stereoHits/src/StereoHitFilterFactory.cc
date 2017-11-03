/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/stereoHits/StereoHitFilterFactory.h>

#include <tracking/trackFindingCDC/filters/stereoHits/StereoHitTruthVarSet.h>
#include <tracking/trackFindingCDC/filters/stereoHits/StereoHitVarSet.h>

#include <tracking/trackFindingCDC/filters/base/MVAFilter.icc.h>

#include <tracking/trackFindingCDC/filters/base/AllFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/RandomFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/ChoosableFromVarSetFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/TruthVarFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.icc.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  using AllStereoHitFilter = AllFilter<BaseStereoHitFilter>;
  using NoneStereoHitFilter = NoneFilter<BaseStereoHitFilter>;
  using RandomStereoHitFilter = RandomFilter<BaseStereoHitFilter>;
  using MCStereoHitFilter = TruthVarFilter<StereoHitTruthVarSet>;
  using SimpleStereoHitFilter = ChoosableFromVarSetFilter<StereoHitVarSet>;
  using RecordingStereoHitFilter =
    RecordingFilter<VariadicUnionVarSet<StereoHitTruthVarSet, StereoHitVarSet>>;
  using MVAStereoHitFilter = MVAFilter<StereoHitVarSet>;
}

template class TrackFindingCDC::FilterFactory<BaseStereoHitFilter>;

StereoHitFilterFactory::StereoHitFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string StereoHitFilterFactory::getIdentifier() const
{
  return "StereoHit";
}

std::string StereoHitFilterFactory::getFilterPurpose() const
{
  return "Stereo hit to track combination filter for adding the hit.";
}

std::map<std::string, std::string>
StereoHitFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no hit track combination is valid."},
    {"all", "set all hits as good."},
    {"random", "returns a random weight (for testing)."},
    {"truth", "monte carlo truth."},
    {"simple", "give back a weight based on very simple variables you can give as a parameter."},
    {"recording", "record variables to a TTree."},
    {"mva", "use the trained BDT."},
  };
}

std::unique_ptr<BaseStereoHitFilter>
StereoHitFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return std::make_unique<NoneStereoHitFilter>();
  } else if (filterName == "all") {
    return std::make_unique<AllStereoHitFilter>();
  } else if (filterName == "random") {
    return std::make_unique<RandomStereoHitFilter>();
  } else if (filterName == "simple") {
    return std::make_unique<SimpleStereoHitFilter>();
  } else if (filterName == "truth") {
    return std::make_unique<MCStereoHitFilter>();
  } else if (filterName == "recording") {
    return std::make_unique<RecordingStereoHitFilter>("StereoHitFilter.root");
  } else if (filterName == "mva") {
    return std::make_unique<MVAStereoHitFilter>("tracking/data/trackfindingcdc_StereoHitFilter.xml");
  } else {
    return Super::create(filterName);
  }
}
