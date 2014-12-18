/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Marco Staric                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/RelationsObject.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <algorithm>
#include <bitset>

namespace genfit { class Track; }

namespace Belle2 {
  /** Class that bundles various TrackFitResults.
   *
   *  As the genfit::Track has a lot of information that is only necessary during the fit,
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
    Track() {
      std::fill(m_trackFitIndices, m_trackFitIndices + Const::chargedStableSet.size(), -1);
    }

    /** Destructor.
     */
    ~Track() {}

    /** Access to TrackFitResults.
     *
     *  Currently it is foreseen to have the logic, which decides, which result is returned in case
     *  of unfitted hypotheses and failed fits in this getter function.
     *  This decision is based on the idea to use the negative index-array values for communicating
     *  reasons for the unavailability of the fit with the correct hypothesis.
     *
     *  @param chargedStable   Determines the particle for which you want to get the best available fit hypothesis.
     *  @return TrackFitResult for fit with particle hypothesis given by ParticleCode.
     *  @sa TrackFitResult
     */
    const TrackFitResult* getTrackFitResult(const Const::ChargedStable& chargedStable) const {
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

    /** Set an index (for positive values) or unavailability-code (with negative values) for a specific mass hypothesis.
     *
     *  The TrackFitResult itself should be saved separately in the DataStore.
     *
     *  @param chargedStable  Determines the hypothesis for which you want to store the index or unavailability-code.
     */
    void setTrackFitResultIndex(const Const::ChargedStable& chargedStable, short index) {
      m_trackFitIndices[chargedStable.getIndex()] = index;
    }

  private:
    /** Index list of the TrackFitResults associated with this Track. */
    short int m_trackFitIndices[Const::ChargedStable::c_SetSize];

    /** This shall save a key with which sets the Track is compatible. */
    //std::bitset<48>

    /** Quality estimator for */
    //short mass hypothesis for which the quality estimator was done.

    /** Second version; doesn't store the name of the TrackFitResults, as default will be taken.
     */
    ClassDef(Track, 3);
  };
}
