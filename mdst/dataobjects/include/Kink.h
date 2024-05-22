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
  /**
   *  Object holding information for Kinks.
   *
   *  This object holds the indices of the Tracks used to create the Kink, the
   *  the indices of the TrackFitResults belonging to that Tracks [with material
   *  effects and hits as appropriate for the IP (mother) and the vertex (mother and daughter)],
   *  the fitted vertex position, and flag of the filter used to reconstruct Kink.
   *
   */
  class Kink : public RelationsObject {
  public:
    /** Constructor without arguments; needed for I/O. */
    Kink();

    /**
     * Constructor taking two pairs of tracks and trackFitResults, the fitted vertex coordinates, and filter flag.
     * @param trackPairMother a pair of mother particle Belle2::Track and a pair of Belle2::TrackFitResult,
     * the first Belle2::TrackFitResult is determined at IP, and the second one is determined at the kink vertex
     * @param trackPairDaughter a pair of daughter particle Belle2::Track and Belle2::TrackFitResult determined
     * at the kink vertex
     * @param vertexX X coordinate of kink vertex
     * @param vertexY Y coordinate of kink vertex
     * @param vertexZ Z coordinate of kink vertex
     * @param filterFlag a flag containing the following information:
     * the first digit is a flag of filter used to preselect the kink candidate (from 1 to 6);
     * the second and third digits form a flag of the two tracks combined fit result (from 0 to 19);
     * the thousands show the number of reassigned hits between tracks (from 0 to 32);
     * the sign shows from which track the hits were taken (- from daughter, + from mother).
     * The content of the filterFlag may change, please, refer for the details to kinkFitter.cc
     */
    Kink(const std::pair<const Belle2::Track*, std::pair<const Belle2::TrackFitResult*, const Belle2::TrackFitResult*> >&
         trackPairMother,
         const std::pair<const Belle2::Track*, const Belle2::TrackFitResult*>& trackPairDaughter,
         Double32_t vertexX, Double32_t vertexY, Double32_t vertexZ, short filterFlag);

    /** Get mother Track.*/
    Track* getMotherTrack() const;

    /** Get daughter Track.*/
    Track* getDaughterTrack() const;

    /** Get indices of the mother Track. */
    short getMotherTrackIndex() const
    {
      return m_trackIndexMother;
    }

    /** Get indices of the mother Track. */
    short getDaughterTrackIndex() const
    {
      return m_trackIndexDaughter;
    }

    /** Get the TrackFitResult of mother at the starting point.*/
    TrackFitResult* getMotherTrackFitResultStart() const;

    /** Get the TrackFitResult of mother at the ending point.*/
    TrackFitResult* getMotherTrackFitResultEnd() const;

    /** Get the TrackFitResult of daughter.*/
    TrackFitResult* getDaughterTrackFitResult() const;

    /** Get index of the TrackFitResult of mother at the starting point. */
    short getTrackFitResultIndexMotherStart() const
    {
      return m_trackFitResultIndexMotherStart;
    }

    /** Get index of the TrackFitResult of mother at the ending point. */
    short getTrackFitResultIndexMotherEnd() const
    {
      return m_trackFitResultIndexMotherEnd;
    }

    /** Get index of the TrackFitResult of daughter. */
    short getTrackFitResultIndexDaughter() const
    {
      return m_trackFitResultIndexDaughter;
    }

    /** Get the X coordinate of the fitted vertex. */
    Double32_t getFittedVertexX() const { return m_fittedVertexX; }

    /** Get the Y coordinate of the fitted vertex. */
    Double32_t getFittedVertexY() const { return m_fittedVertexY; }

    /** Get the Z coordinate of the fitted vertex. */
    Double32_t getFittedVertexZ() const { return m_fittedVertexZ; }

    /** Get the fitted vertex position. */
    ROOT::Math::XYZVector getFittedVertexPosition() const
    {
      return ROOT::Math::XYZVector(m_fittedVertexX, m_fittedVertexY, m_fittedVertexZ);
    }

    /** Get the filter flag. */
    short getFilterFlag() const
    {
      return m_filterFlag;
    }

  private:
    /** Indicates which mother track was used for this Kink. */
    short m_trackIndexMother;

    /** Indicates which daughter track was used for this Kink. */
    short m_trackIndexDaughter;

    /** Points to the new TrackFitResult of the mother Track at Start. */
    short m_trackFitResultIndexMotherStart;

    /** Points to the new TrackFitResult of the mother Track at End. */
    short m_trackFitResultIndexMotherEnd;

    /** Points to the new TrackFitResult of the daughter Track. */
    short m_trackFitResultIndexDaughter;

    /** The X coordinate of the fitted vertex. */
    Double32_t m_fittedVertexX;

    /** The Y coordinate of the fitted vertex. */
    Double32_t m_fittedVertexY;

    /** The Z coordinate of the fitted vertex. */
    Double32_t m_fittedVertexZ;

    /** The filter flag of the kink. */
    short m_filterFlag;

    /** Macro for ROOTification. */
    ClassDef(Kink, 1);

    friend class FixMergedObjectsModule;
  };
}
