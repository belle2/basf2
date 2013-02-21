/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>
#include <tracking/dataobjects/TrackFitResult.h>

class GFTrack;

namespace Belle2 {



  /** Class that bundles various TrackFitResults.
   *
   *  As the GFTrack has a lot of information that is only necessary during the fit,
   *  this is a much stripped down version containing only indices of TrackFitResults
   *  (which is what remains of the various TrackReps, that Genfit might use during the fit).
   */
  // template <typename T>
  class TrackGen : public TObject {
  public:
    /** Constuctor without arguments; needed for I/O.
     */
    TrackGen(): m_trackFitResultsName("TrackFitResults") {
      std::fill(m_trackFitIndices, m_trackFitIndices + 5, -1);
    }

    /** Construct a Track using a GFTrack.
     */
    //TrackGen(const GFTrack& gfTrack);

    /** Destructor.
     */
    ~TrackGen() {}

    /** Access to TrackFitResults.
     *
     *  @return TrackFitResult for fit with particle hypothesis given by ParticleCode.
     *  @sa ParticleCode, TrackFitResult
     */
    const TrackFitResult* getTrackFitResult(const Const::ChargedStable chargedStable) const {
      StoreArray<TrackFitResult> trackFitResults(m_trackFitResultsName);
      return trackFitResults[m_trackFitIndices[chargedStable.getIndex()]];
    }
    /*const TrackFitResult* getTrackFitResult(const T& AParticleHypothesis) const{
      return m_trackFitResults[m_trackFitIndices[AParticleHypothesis.getIndex()]];
    } */

    /** The TrackFitResult itself should be saved separately in the DataStore. */
    void setTrackFitResultIndex(const Const::ChargedStable chargedStable, short index) {
      m_trackFitIndices[chargedStable.getIndex()] = index;
    }

  private:
    /** Index list of the TrackFitResults associated with this Track.
     *
     *  Experience will tell us, how many fits we really have to associate do for one Track.
     *  Currently assuming a maximum of 5 (electron, muon, pion, kaon, proton).
     *  \sa ChargedStable
     */
    short int m_trackFitIndices[Const::ChargedStable::c_SetSize];

    /** Name of array in which corresponding TrackFitResults are saved.
     */
    std::string m_trackFitResultsName;

//    StoreArray<TrackFitResult> m_trackFitResults; //!


    /** First version. */
    ClassDef(TrackGen, 1);
  };
}
