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
   *  This object holds the indices of the Tracks used to create the V0, the
   *  the indices of the TrackFitResults belonging to that Tracks (with material
   *  effects and hits as appropriate for the point of the intersection of the
   *  Tracks), and the fitted vertex position (to be used as a starting point
   *  for vertex fitting at analysis level).
   *
   *  For additional information see [BELLE2-NOTE-TE-2015-034].
   *  --> https://docs.belle2.org/record/307
   *  Please note that the note was written before introducing the vertex
   *  position in this object.
   */
  class V0 : public RelationsObject {
  public:
    /** Constructor without arguments; needed for I/O. */
    V0();

    /** Constructor taking two pairs of tracks and trackFitResults, and the fitted vertex coordinates. */
    V0(const std::pair<const Belle2::Track*, const Belle2::TrackFitResult*>& trackPairPositive,
       const std::pair<const Belle2::Track*, const Belle2::TrackFitResult*>& trackPairNegative,
       double vertexX, double vertexY, double vertexZ);

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

    /** Get the X coordinate of the fitted vertex. */
    double getFittedVertexX() const { return m_fittedVertexX; }

    /** Get the Y coordinate of the fitted vertex. */
    double getFittedVertexY() const { return m_fittedVertexY; }

    /** Get the Z coordinate of the fitted vertex. */
    double getFittedVertexZ() const { return m_fittedVertexZ; }

    /** Get the fitted vertex position. */
    ROOT::Math::XYZVector getFittedVertexPosition() const { return ROOT::Math::XYZVector(m_fittedVertexX, m_fittedVertexY, m_fittedVertexZ); }

  private:
    /** Indicates which positively charged track was used for this V0. */
    short m_trackIndexPositive;

    /** Indicates which negatively charged track was used for this V0. */
    short m_trackIndexNegative;

    /** Points to the new TrackFitResult of the positive Track. */
    short m_trackFitResultIndexPositive;

    /** Points to the new TrackFitResult of the positive Track. */
    short m_trackFitResultIndexNegative;

    /** The X coordinate of the fitted vertex. */
    Double32_t m_fittedVertexX;

    /** The Y coordinate of the fitted vertex. */
    Double32_t m_fittedVertexY;

    /** The Z coordinate of the fitted vertex. */
    Double32_t m_fittedVertexZ;

    /** Macro for ROOTification. */
    ClassDef(V0, 4);

    friend class FixMergedObjectsModule;
  };
}
