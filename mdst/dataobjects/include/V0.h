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
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>
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

    /** Deprecated
     *  Constructor, just for compatibility reasons.
     */
    V0(const std::pair<Belle2::Track*, Belle2::TrackFitResult*>& trackPairPositive,
       const std::pair<Belle2::Track*, Belle2::TrackFitResult*>& trackPairNegative);

    /** Constructor taking a pair of a Track and a TrackFitResult.
     *
     *  By taking such a pair the likelihood is minimized, that later people give the Track and the
     *  TrackFitResult in mixed order to the V0.
     *  @param trackPairPositive  A pair of the Track forming one part of the V0 and the result of the refitting.
     *  @param trackPairNegative  @sa trackPairPositive
     *  //TODO: Use a tuple<Track, TFR, TFR> as soon as it becomes available
     */
    V0(const std::pair<Belle2::Track*, std::pair<Belle2::TrackFitResult*, Belle2::TrackFitResult*> >& trackPairPositive,
       const std::pair<Belle2::Track*, std::pair<Belle2::TrackFitResult*, Belle2::TrackFitResult*> >& trackPairNegative);

    /** Get pair of Tracks, that are part of the V0 particle.*/
    std::pair<Track*, Track*> getTrackPtrs() {
      StoreArray<Track> tracks;
      return std::make_pair(tracks[m_trackIndexPositive], tracks[m_trackIndexNegative]);
    }

    /** Get indices of the Tracks, that are part of the V0 particle. */
    std::pair<short, short> getTrackIndices() {
      return std::make_pair(m_trackIndexPositive, m_trackIndexNegative);
    }

    /** Deprecated
     *  Getter just for compatibility reason.
     */
    std::pair<TrackFitResult*, TrackFitResult*> getTrackFitResultPtrs() {
      B2WARNING("Deprecated. Ask for the hypothesis explicit.")
      StoreArray<TrackFitResult> trackFitResults;
      return std::make_pair(trackFitResults[m_trackFitResultIndexPiPlus], trackFitResults[m_trackFitResultIndexPiMinus]);
    }

    /** Get pair of TrackFitResults, that are part of the V0 particle for a given V0 hypothesis.
     * @param particleType Kshort, Lambda or Anti-Lambda
     * @return
     * */
    std::pair<TrackFitResult*, TrackFitResult*> getTrackFitResultPtrs(const Const::ParticleType& particleType) {
      StoreArray<TrackFitResult> trackFitResults;
      if (particleType == Const::Kshort) return std::make_pair(trackFitResults[m_trackFitResultIndexPiPlus], trackFitResults[m_trackFitResultIndexPiMinus]);
      if (particleType == Const::lambda) return std::make_pair(trackFitResults[m_trackFitResultIndexProton], trackFitResults[m_trackFitResultIndexPiMinus]);
      if (particleType == Const::antiLambda) return std::make_pair(trackFitResults[m_trackFitResultIndexPiPlus], trackFitResults[m_trackFitResultIndexAProton]);
      else {
        B2ERROR("No TrackFitResults for this V0 hypothesis.");
        return std::make_pair(trackFitResults[-1], trackFitResults[-1]);
      }
    }

    /** Get indices of the TrackFitResults, that are part of the V0 particle for a given V0 hypothesis.
     * @param particleType particleType Kshort, Lambda or Anti-Lambda
     * @return
     */
    std::pair<short, short> getTrackFitResultIndices(const Const::ParticleType& particleType) {
      if (particleType == Const::Kshort) return std::make_pair(m_trackFitResultIndexPiPlus, m_trackFitResultIndexPiMinus);
      if (particleType == Const::lambda) return std::make_pair(m_trackFitResultIndexProton, m_trackFitResultIndexPiMinus);
      if (particleType == Const::antiLambda) return std::make_pair(m_trackFitResultIndexPiPlus, m_trackFitResultIndexAProton);
      else {
        B2ERROR("No TrackFitResults for this V0 hypothesis.");
        return std::make_pair(-1, -1);
      }
    }

  private:
    /** Indicates which positively charged track was used for this V0. */
    short m_trackIndexPositive;

    /** Indicates which negatively charged track was used for this V0. */
    short m_trackIndexNegative;

    /** Points to the new TrackFitResult of the positive Track under pion hypothesis.
     *
     *  As the V0 particle may have traveled through material and may have added hits,
     *  that are unlikely really part of the track, a new TrackFitResult has to be created,
     *  that contains the information extrapolated to the area, where the two tracks from
     *  this V0 intersect.
     */
    short m_trackFitResultIndexPiPlus;

    /** Points to the new TrackFitResult of the positive Track under proton hypothesis.
     * @sa m_trackFitResultIndexPiPlus
     */
    short m_trackFitResultIndexProton;

    /** Points to the new TrackFitResult of the negative Track under pion hypothesis.
     *  @sa m_trackFitResultIndexPiPlus
     */
    short m_trackFitResultIndexPiMinus;

    /** Points to the new TrackFitResult of the negative Track under proton hypothesis.
     *  @sa m_trackFitResultIndexPiPlus
     */
    short m_trackFitResultIndexAProton;

    /** Macro for ROOTification. */
    ClassDef(V0, 2);
  };
}
