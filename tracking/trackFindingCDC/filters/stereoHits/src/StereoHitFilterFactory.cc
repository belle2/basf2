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

using namespace Belle2;
using namespace TrackFindingCDC;

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
    {"tmva", "use the trained BDT."},
  };
}

std::unique_ptr<BaseStereoHitFilter>
StereoHitFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return makeUnique<BaseStereoHitFilter>();
  } else if (filterName == "all") {
    return makeUnique<AllStereoHitFilter>();
  } else if (filterName == "random") {
    return makeUnique<RandomStereoHitFilter>();
  } else if (filterName == "simple") {
    return makeUnique<SimpleStereoHitFilter>();
  } else if (filterName == "truth") {
    return makeUnique<MCStereoHitFilter>();
  } else if (filterName == "recording") {
    return makeUnique<RecordingStereoHitFilter>("StereoHit.root");
  } else if (filterName == "tmva") {
    return makeUnique<TMVAStereoHitFilter>("StereoHit");
  } else {
    return Super::create(filterName);
  }
}
