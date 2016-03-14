/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Markus Prim                                 *
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

    /** Constructor taking two pairs of tracks and trackFitResults. */
    V0(const std::pair<Belle2::Track*, Belle2::TrackFitResult*>& trackPairPositive,
       const std::pair<Belle2::Track*, Belle2::TrackFitResult*>& trackPairNegative);

    /** Get pair of  yhe Tracks, that are part of the V0 particle.*/
    std::pair<Track*, Track*> getTracks()
    {
      StoreArray<Track> tracks;
      return std::make_pair(tracks[m_trackIndexPositive], tracks[m_trackIndexNegative]);
    }

    /** Get indices of the Tracks, that are part of the V0 particle. */
    std::pair<short, short> getTrackIndices()
    {
      return std::make_pair(m_trackIndexPositive, m_trackIndexNegative);
    }

    /** Get pair of the TrackFitResults, that are part of the V0 particle.*/
    std::pair<TrackFitResult*, TrackFitResult*> getTrackFitResults()
    {
      StoreArray<TrackFitResult> trackFitResults;
      return std::make_pair(trackFitResults[m_trackFitResultIndexPositive], trackFitResults[m_trackFitResultIndexNegative]);
    }

    /** Get indices of the TrackFitResults, that are part of the V0 particle. */
    std::pair<short, short> getTrackFitResultIndices()
    {
      return std::make_pair(m_trackFitResultIndexPositive, m_trackFitResultIndexNegative);
    }

    /** Get the hypothesis under which the V0 particle was created.
     * This does not use an MC information, but just checks the particle type in the corresponding TrackFitResults.*/
    Const::ParticleType getV0Hypothesis();

  private:
    /** Indicates which positively charged track was used for this V0. */
    short m_trackIndexPositive;

    /** Indicates which negatively charged track was used for this V0. */
    short m_trackIndexNegative;

    /** Points to the new TrackFitResult of the positive Track. */
    short m_trackFitResultIndexPositive;

    /** Points to the new TrackFitResult of the positive Track. */
    short m_trackFitResultIndexNegative;

    /** Macro for ROOTification. */
    ClassDef(V0, 3);
  };
}
