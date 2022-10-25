/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <mdst/dataobjects/Track.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <sstream>

using namespace Belle2;

const TrackFitResult* Track::getTrackFitResultByName(const Const::ChargedStable& chargedStable,
                                                     const std::string trackFitResultsName) const
{
  const auto trackFitResultArrayIndex = m_trackFitIndices[chargedStable.getIndex()];
  if (trackFitResultArrayIndex < 0) {
    B2DEBUG(20, "TrackFitResult for the requested hypothesis is not set. Returning a nullptr instead.");
    return nullptr;
  }

  StoreArray<TrackFitResult> trackFitResults(trackFitResultsName);
  return trackFitResults[trackFitResultArrayIndex];
}

const TrackFitResult* Track::getTrackFitResult(const Const::ChargedStable& chargedStable) const
{
  return getTrackFitResultByName(chargedStable, "");
}


unsigned int Track::getNumberOfFittedHypotheses() const
{
  return getValidIndices().size();
}


std::vector<Track::ChargedStableTrackFitResultPair> Track::getTrackFitResultsByName(const std::string trackFitResultsName) const
{
  StoreArray<TrackFitResult> trackFitResults(trackFitResultsName);
  std::vector<Track::ChargedStableTrackFitResultPair> result;

  const auto validParticleIndices = getValidIndices();

  // extract the particle class and trackfitresult pointer for each
  // stored hypothesis
  for (auto  particleIndex : validParticleIndices) {
    const auto indexInStoreArray = m_trackFitIndices[particleIndex];
    result.emplace_back(std::make_pair(Const::ChargedStable(Const::chargedStableSet.at(particleIndex)),
                                       trackFitResults[indexInStoreArray]));
  }

  return result;
}

std::vector<Track::ChargedStableTrackFitResultPair> Track::getTrackFitResults() const
{
  return getTrackFitResultsByName("");
}

std::vector < short int> Track::getValidIndices() const
{
  std::vector <short int> resultParticleIndex;

  short int i = 0;
  for (const auto& hyp : m_trackFitIndices) {
    if (hyp != -1) {
      resultParticleIndex.push_back(i);
    }
    i++;
  }

  return resultParticleIndex;
}

const TrackFitResult* Track::getTrackFitResultWithClosestMassByName(const Const::ChargedStable& requestedType,
    const std::string trackFitResultsName) const
{
  // make sure at least one hypothesis exist. No B2 Track should exist which does not have at least
  // one hypothesis
  B2ASSERT("Belle2::Track must always have at least one successfully fitted hypothesis.", getNumberOfFittedHypotheses() > 0);

  // find fitted hypothesis which is closest to the mass of our requested particle type
  auto allFitRes = getTrackFitResultsByName(trackFitResultsName);

  // sort so the closest mass hypothesis fit in the first entry of the vector
  auto bestMassFit = std::min_element(allFitRes.begin(), allFitRes.end(), [requestedType](auto & a, auto & b) {
    const auto massDiffA = std::abs(a.first.getMass() - requestedType.getMass());
    const auto massDiffB = std::abs(b.first.getMass() - requestedType.getMass());

    return massDiffA < massDiffB;
  });

  if (std::isnan(bestMassFit->second->getPValue())) {
    return nullptr;
  }
  return bestMassFit->second;
}

const TrackFitResult* Track::getTrackFitResultWithClosestMass(const Const::ChargedStable& requestedType) const
{
  return getTrackFitResultWithClosestMassByName(requestedType, "");
}

std::string Track::getInfoHTML() const
{
  std::stringstream out;
  out << "<b>Number of Fitted Hypothesis</b>: " << getNumberOfFittedHypotheses() << "<br>";

  // just output all the TrackFitResult infos.
  size_t count = 1;
  for (auto fitResults : getTrackFitResults()) {
    out << "<p>";
    out << "<br><b>-- Hypothesis " << count << " --</b><br>";
    out << fitResults.second->getInfoHTML();
    out << "</p>";
    count++;
  }
  return out.str();
}
