/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <mdst/dataobjects/Track.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

const TrackFitResult* Track::getTrackFitResult(const Const::ChargedStable& chargedStable) const
{
  StoreArray<TrackFitResult> trackFitResults;
  const auto trackFitResultArrayIndex = m_trackFitIndices[chargedStable.getIndex()];
  if (trackFitResultArrayIndex < 0) {
    B2DEBUG(100, "TrackFitResult for the requested hypothesis is not set. Returning default hypothesis instead.");
    const auto defaultTrackFitResultArrayIndex = m_trackFitIndices[Const::pion.getIndex()];
    if (defaultTrackFitResultArrayIndex < 0) {
      B2DEBUG(100, "TrackFitResult for the default hypothesis is not set. Returning any I can find or nullptr.");
      TrackFitResult* lastTrackFitResult = nullptr;
      for (const auto& arrayIndex : m_trackFitIndices) {
        if (arrayIndex >= 0) lastTrackFitResult = trackFitResults[arrayIndex];
      }
      return lastTrackFitResult;
    } else {
      return trackFitResults[defaultTrackFitResultArrayIndex];
    }
  } else {
    return trackFitResults[trackFitResultArrayIndex];
  }
}

unsigned int Track::getNumberOfFittedHypotheses() const
{
  unsigned int fittedHypothesis = 0;
  for (const auto& hyp : m_trackFitIndices) {
    if (hyp != -1) {
      ++fittedHypothesis;
    }
  }
  return fittedHypothesis;
}
