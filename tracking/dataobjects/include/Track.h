/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko                          *
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
  class Track : public TObject {
  public:
    /** Constuctor.
     */
    Track();

    /** Construct a Track using a GFTrack.
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
    TrackFitResult* getTrackFitResult(const Const::ChargedStable& chargedStable);


  private:
    /** Index list of the TrackFitResults associated with this Track.
     *
     *  Experience will tell us, how many fits we really have to associate do for one Track.
     *  Currently assuming a maximum of 5 (electron, muon, pion, kaon, proton).
     *  \sa ChargedStable
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
