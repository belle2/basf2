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
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/Track.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/datastore/StoreArray.h>
#include <utility>

namespace Belle2 {
  /** Object holding information for V0s.
   *
   *  This object only holds the indices of the Tracks used to create the V0 and
   *  the indices of the TrackFitResults belonging to that Tracks, but material effects
   *  and hits as appropriate for the point of the intersection of the Tracks.
   */
  class V0 : public RelationsObject {
  public:
    /** Constructor without arguments; needed for I/O. */
    V0();

    /** Constructor taking a pair of a Track and a TrackFitResult.
     *
     *  By taking such a pair the likelihood is minimized, that later people give the Track and the
     *  TrackFitResult in mixed order to the V0.
     *  @param trackPairPositive  A pair of the Track forming one part of the V0 and the result of the refitting.
     *  @param trackPairNegative  @sa trackPairPositive
     */
    V0(const std::pair<Track, TrackFitResult>& trackPairPositive,
       const std::pair<Track, TrackFitResult>& trackPairNegative);

    /** Get pair of Tracks, that are part of the V0 particle.*/
    std::pair<Track*, Track*> getTrackPtrs() {
      StoreArray<Track> tracks;
      return std::make_pair(tracks[m_trackIndexPositive], tracks[m_trackIndexNegative]);
    }

    /** Get indices of the Tracks, that are part of the V0 particle. */
    std::pair<short, short> getTrackIndices() {
      return std::make_pair(m_trackIndexPositive, m_trackIndexNegative);
    }

    /** Get pair of TrackFitResults, that are part of the V0 particle.*/
    std::pair<TrackFitResult*, TrackFitResult*> getTrackFitResultPtrs() {
      StoreArray<TrackFitResult> trackFitResults;
      return std::make_pair(trackFitResults[m_trackIndexPositive], trackFitResults[m_trackIndexNegative]);
    }

    /** Get indices of the TrackFitResults, that are part of the V0 particle. */
    std::pair<short, short> getTrackFitResultIndices() {
      return std::make_pair(m_trackFitResultIndexPositive, m_trackFitResultIndexNegative);
    }

  private:
    /** Indicates which positively charged track was used for this V0. */
    short m_trackIndexPositive;

    /** Indicates which negatively charged track was used for this V0. */
    short m_trackIndexNegative;

    /** Points to the new TrackFitResult of the positive Track.
     *
     *  As the V0 particle may have traveled through material and may have added hits,
     *  that are unlikely really part of the track, a new TrackFitResult has to be created,
     *  that contains the information extrapolated to the area, where the two tracks from
     *  this V0 intersect.
     */
    short m_trackFitResultIndexPositive;

    /** Points to the new TrackFitResult of the positive Track.
     *
     *  @sa m_trackFitResultIndexPositive
     */
    short m_trackFitResultIndexNegative;

    /** Macro for ROOTification. */
    ClassDef(V0, 1);
  };
}
