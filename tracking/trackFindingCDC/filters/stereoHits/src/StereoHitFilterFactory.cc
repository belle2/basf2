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

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

std::map<std::string, std::string>
StereoHitFilterFactory::getValidFilterNamesAndDescriptions() const
{
  std::map<std::string, std::string>
  filterNames = Super::getValidFilterNamesAndDescriptions();

  filterNames.insert({
    {"truth", "monte carlo truth."},
    {"all", "set all segments as good."},
    {"none", "no segment track combination is valid."},
    {"recording", "record variables to a TTree."},
    {"random", "returns a random weight (for testing)."},
    {"simple", "give back a weight based on very simple variables you can give as a parameter."}
  });
  return filterNames;
}

std::unique_ptr<BaseStereoHitFilter>
StereoHitFilterFactory::create(const std::string& filterName) const
{
  if (filterName == string("none")) {
    return std::unique_ptr<BaseStereoHitFilter>(new BaseStereoHitFilter());
  } else if (filterName == string("truth")) {
    return std::unique_ptr<BaseStereoHitFilter>(new MCStereoHitFilter());
  } else if (filterName == string("all")) {
    return std::unique_ptr<BaseStereoHitFilter>(new AllStereoHitFilter());
  } else if (filterName == string("recording")) {
    return std::unique_ptr<BaseStereoHitFilter>(new RecordingStereoHitFilter("StereoHit.root"));
  } else if (filterName == string("simple")) {
    return std::unique_ptr<BaseStereoHitFilter>(new SimpleStereoHitFilter());
  } else if (filterName == string("random")) {
    return std::unique_ptr<BaseStereoHitFilter>(new RandomStereoHitFilter());
  } else {
    return Super::create(filterName);
  }
}
