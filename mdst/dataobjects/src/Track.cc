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
  unsigned int fittedHypothesis = 0;
  for (const auto& hyp : m_trackFitIndices) {
    if (hyp != -1) {
      ++fittedHypothesis;
    }
  }
  return fittedHypothesis;
}
