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
  if (m_trackFitIndices[chargedStable.getIndex()] < 0) {
    B2DEBUG(100, "Attempt to access an unset TrackFitResult");
    //ULTRA PRELIMINARY
    short int index = -1; // MS: better to set to invalid value and test again after
    for (unsigned int ii = 0; ii < Const::chargedStableSet.size(); ii++) {
      if (m_trackFitIndices[ii] >= 0) {
        index = m_trackFitIndices[ii];
      }
    }
    if (index < 0) return 0; // MS: just in case and to be sure not to get garbage
    StoreArray<TrackFitResult> trackFitResults;
    return trackFitResults[index];
  }
  StoreArray<TrackFitResult> trackFitResults;
  return trackFitResults[m_trackFitIndices[chargedStable.getIndex()]];
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
