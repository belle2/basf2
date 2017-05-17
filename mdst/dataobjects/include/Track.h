/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Marco Staric, Nils Braun                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <framework/gearbox/Const.h>
#include <framework/datastore/RelationsObject.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <algorithm>

namespace Belle2 {
  class TrackFitResult;
  /** Class that bundles various TrackFitResults.
   *
   *  As the RecoTrack has a lot of information that is only necessary during the fit,
   *  this is a much stripped down version containing only indices of TrackFitResults
   *  (which is what remains of the various TrackReps, that Genfit might use during the fit).
   */
  class Track : public RelationsObject {
  public:
    /** Constructor without arguments; needed for I/O.
     *
     *  The array with the indices for the TrackFitResults is initialized with -1,
     *  which is an invalid index.
     */
    Track()
    {
      std::fill(m_trackFitIndices, m_trackFitIndices + Const::chargedStableSet.size(), -1);
    }

    /** Access to TrackFitResults.
     *
     * This tries to return the TrackFitResult for the requested track hypothesis. If the requested track hypothesis is
     * not available, we return a nullptr.
     *
     * TODO: Do something special if we did not even try to fit!
     *
     *  @param chargedStable   Determines the particle for which you want to get the best available fit hypothesis.
     *  @return TrackFitResult for fit with particle hypothesis given by ParticleCode or a nullptr, if no result is
     *          available.
     *  @sa TrackFitResult
     */
    const TrackFitResult* getTrackFitResult(const Const::ChargedStable& chargedStable) const;

    /** Set an index (for positive values) or unavailability-code (with negative values) for a specific mass hypothesis.
     *
     *  The TrackFitResult itself should be saved separately in the DataStore.
     *
     *  @param chargedStable  Determines the hypothesis for which you want to store the index or unavailability-code.
     */
    void setTrackFitResultIndex(const Const::ChargedStable& chargedStable, short index)
    {
      m_trackFitIndices[chargedStable.getIndex()] = index;
    }

    /** Returns the number of fitted hypothesis which are stored in this track. */
    unsigned int getNumberOfFittedHypotheses() const;

  private:
    /** Index list of the TrackFitResults associated with this Track. */
    short int m_trackFitIndices[Const::ChargedStable::c_SetSize];

    ClassDef(Track, 3); // Class that bundles various TrackFitResults.
  };
}
