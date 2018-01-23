/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Nils Braun                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mdst/dataobjects/Track.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

const TrackFitResult* Track::getTrackFitResult(const Const::ChargedStable& chargedStable) const
{
  const auto trackFitResultArrayIndex = m_trackFitIndices[chargedStable.getIndex()];
  if (trackFitResultArrayIndex < 0) {
    B2DEBUG(100, "TrackFitResult for the requested hypothesis is not set. Returning a nullptr instead.");
    return nullptr;
  }

  StoreArray<TrackFitResult> trackFitResults;
  return trackFitResults[trackFitResultArrayIndex];
}

unsigned int Track::getNumberOfFittedHypotheses() const
{
  return getValidIndices().size();
}


std::vector<Track::ChargedStableTrackFitResultPair> Track::getTrackFitResults() const
{
  StoreArray<TrackFitResult> trackFitResults;
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

const TrackFitResult* Track::getTrackFitResultWithClosestMass(const Const::ChargedStable& requestedType) const
{
  // make sure at least one hypothesis exist. No B2 Track should exist which does not have at least
  // one hypothesis
  B2ASSERT("Belle2::Track must always have at least one successfully fitted hypothesis.", getNumberOfFittedHypotheses() > 0);

  // find fitted hypothesis which is closest to the mass of our requested particle type
  auto allFitRes = getTrackFitResults();

  // sort so the closest mass hypothesis fit in the first entry of the vector
  auto bestMassFit = std::min_element(allFitRes.begin(), allFitRes.end(), [requestedType](auto & a, auto & b) {
    const auto massDiffA = std::abs(a.first.getMass() - requestedType.getMass());
    const auto massDiffB = std::abs(b.first.getMass() - requestedType.getMass());

    return massDiffA < massDiffB;
  });

  return bestMassFit->second;
}
