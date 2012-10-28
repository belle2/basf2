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

#include <tracking/dataobjects/TrackFitResult.h>
#include <framework/datastore/StoreArray.h>

#include <GFTrack.h>

#include <string>

namespace Belle2 {
  class ParticleCode;

  /** Class that bundles various TrackFitResults.
   *
   *  As the GFTrack has a lot of information that is only necessary during the fit,
   *  this is a much stripped down version containing only indices of TrackFitResults
   *  (which is what remains of the various TrackReps, that Genfit might use during the fit).
   */
  class Track : public TObject {
  public:
    /** Constuctor.
     */
    Track();

    /** Construct an Track using a GFTrack.
     */
    Track(const GFTrack& gfTrack);

    /** Destructor.
     */
    ~Track() {}

    /** Access to TrackFitResults.
     *
     *  @return TrackFitResult for fit with particle hypothesis given by ParticleCode.
     *  @sa ParticleCode, TrackFitResult
     */
    TrackFitResult* getTrackFitResult(const ParticleCode& particleCode);


  private:
    /** Index list of the TrackFitResults associated with this Track.
     *
     *  Experience will tell us, how many fits we really have to associate do for one Track.
     *  Currently assuming a maximum of 5 (electron, muon, pion, kaon, proton)
     *  \sa ParticleCode
     */
    unsigned short int m_trackFitIndices[5];

    /** Name of array in which corresponding TrackFitResults are saved.
     */
    std::string m_trackFitResultsName;

    StoreArray<TrackFitResult> m_trackFitResults;


    /** First version. */
    ClassDef(Track, 0);
  };
}
