/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/Track.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>
#include <utility>

namespace Belle2 {
  /** Object holding information for V0s.
   *
   *  This object only holds the indices of the Tracks used to create the V0 and
   *  the indices of the TrackFitResults belonging to that Tracks, but material effects
   *  and hits as appropriate for the point of the intersection of the Tracks.
   *
   *  For additional information see [BELLE2-NOTE-TE-2015-034].
   *  --> https://d2comp.kek.jp/record/307/files/BELLE2-NOTE-TE-2015-034.pdf
   */
  class V0 : public RelationsObject {
  public:
    /** Constructor without arguments; needed for I/O. */
    V0();

    /** Constructor taking two pairs of tracks and trackFitResults. */
    V0(const std::pair<const Belle2::Track*, const Belle2::TrackFitResult*>& trackPairPositive,
       const std::pair<const Belle2::Track*, const Belle2::TrackFitResult*>& trackPairNegative);

    /** Get pair of  yhe Tracks, that are part of the V0 particle.*/
    std::pair<Track*, Track*> getTracks() const
    {
      StoreArray<Track> tracks;
      return std::make_pair(tracks[m_trackIndexPositive], tracks[m_trackIndexNegative]);
    }

    /** Get indices of the Tracks, that are part of the V0 particle. */
    std::pair<short, short> getTrackIndices() const
    {
      return std::make_pair(m_trackIndexPositive, m_trackIndexNegative);
    }

    /** Get pair of the TrackFitResults, that are part of the V0 particle.*/
    std::pair<TrackFitResult*, TrackFitResult*> getTrackFitResults() const
    {
      StoreArray<TrackFitResult> trackFitResults;
      return std::make_pair(trackFitResults[m_trackFitResultIndexPositive], trackFitResults[m_trackFitResultIndexNegative]);
    }

    /** Get indices of the TrackFitResults, that are part of the V0 particle. */
    std::pair<short, short> getTrackFitResultIndices() const
    {
      return std::make_pair(m_trackFitResultIndexPositive, m_trackFitResultIndexNegative);
    }

    /** Get the hypothesis under which the V0 particle was created.
     * This does not use an MC information, but just checks the particle type in the corresponding TrackFitResults.*/
    Const::ParticleType getV0Hypothesis() const;

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
