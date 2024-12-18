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
   *  This object holds the indices of the Tracks used to create the Kink,
   *  the indices of the TrackFitResults belonging to that Tracks [with material
   *  effects and hits as appropriate for the IP (mother) and the vertex (mother and daughter)],
   *  the fitted vertex position, and flag, indicating the filter used to reconstruct Kink,
   *  holding information about the combined fit of two tracks, and storing number of reassigned hits
   *  between the tracks.
   *
   */
  class Kink : public RelationsObject {
  public:
    /** Constructor without arguments; needed for I/O. Should not be used to create Kinks! */
    Kink();

    /**
     * Constructor taking two pairs of tracks and trackFitResults, the fitted vertex coordinates, and filter flag.
     * @param trackPairMother a pair of mother particle `Belle2::Track` and a pair of `Belle2::TrackFitResult`,
     * the first Belle2::TrackFitResult is determined at IP, and the second one is determined at the kink vertex
     * @param trackPairDaughter a pair of daughter particle `Belle2::Track` and `Belle2::TrackFitResult` determined
     * at the kink vertex
     * @param vertexX X coordinate of kink vertex
     * @param vertexY Y coordinate of kink vertex
     * @param vertexZ Z coordinate of kink vertex
     * @param filterFlag a flag containing the following information:
     * the 1st digit is a flag of filter used to preselect the kink candidate (from 1 to 5);
     * the 2nd and 3rd digits form a flag of the two tracks combined fit result (from 0 to 15, 18, and 19)
     * if the first digit is 1 or 2; if the first digit is 3 to 5, then the second digit is equal to 1 when
     * the distance between daughter and mother track at vertex bigger than the cut in `KinkFitter`;
     * the 4th and 5th digits together show the number of reassigned hits between tracks (from 0 to 32);
     * the sign shows from which track the hits were taken (- from daughter, + from mother).
     * The content of the filterFlag may change, please, refer for the details to `KinkFitter.cc`.
     */
    Kink(const std::pair<const Belle2::Track*,
         std::pair<const Belle2::TrackFitResult*, const Belle2::TrackFitResult*> >& trackPairMother,
         const std::pair<const Belle2::Track*, const Belle2::TrackFitResult*>& trackPairDaughter,
         const Double32_t vertexX, const Double32_t vertexY, const Double32_t vertexZ, const short filterFlag);

    /** Get mother `Track`.*/
    Track* getMotherTrack() const;

    /** Get daughter `Track` (it is the same as mother's in case of track splitting).*/
    Track* getDaughterTrack() const;

    /** Get index of the mother `Track`. */
    short getMotherTrackIndex() const
    {
      return m_trackIndexMother;
    }

    /** Get index of the daughter `Track` (it is the same as mother's in case of track splitting). */
    short getDaughterTrackIndex() const
    {
      return m_trackIndexDaughter;
    }

    /** Get the `TrackFitResult` of mother at the starting point.*/
    TrackFitResult* getMotherTrackFitResultStart() const;

    /** Get the `TrackFitResult` of mother at the ending point.*/
    TrackFitResult* getMotherTrackFitResultEnd() const;

    /** Get the `TrackFitResult` of daughter at the starting point.*/
    TrackFitResult* getDaughterTrackFitResult() const;

    /** Get index of the `TrackFitResult` of mother at the starting point. */
    short getTrackFitResultIndexMotherStart() const
    {
      return m_trackFitResultIndexMotherStart;
    }

    /** Get index of the `TrackFitResult` of mother at the ending point. */
    short getTrackFitResultIndexMotherEnd() const
    {
      return m_trackFitResultIndexMotherEnd;
    }

    /** Get index of the `TrackFitResult` of daughter at the starting point. */
    short getTrackFitResultIndexDaughter() const
    {
      return m_trackFitResultIndexDaughter;
    }

    /** Get the X coordinate of the fitted kink vertex. */
    Double32_t getFittedVertexX() const { return m_fittedVertexX; }

    /** Get the Y coordinate of the fitted kink vertex. */
    Double32_t getFittedVertexY() const { return m_fittedVertexY; }

    /** Get the Z coordinate of the fitted kink vertex. */
    Double32_t getFittedVertexZ() const { return m_fittedVertexZ; }

    /** Get the fitted kink vertex position. */
    ROOT::Math::XYZVector getFittedVertexPosition() const
    {
      return ROOT::Math::XYZVector(m_fittedVertexX, m_fittedVertexY, m_fittedVertexZ);
    }

    /**
     * Get the filter flag.
     * For the content of the flag, refer to the documentation of Kink::m_filterFlag.
     * Use this function only in case one of the content getters for some reason returns wrong values.
     */
    short getFilterFlag() const
    {
      return m_filterFlag;
    }

    /**
     * Get the flag of prefilter with which kink track pair (or track in case of splitting) was selected.
     * For details, refer to the documentation of Kink::m_filterFlag.
     */
    short getPrefilterFlag() const
    {
      return abs(m_filterFlag) % 10;
    }

    /**
     * Get the flag containing information about combined fit for track pairs.
     * For details, refer to the documentation of Kink::m_filterFlag.
     */
    short getCombinedFitResultFlag() const
    {
      return (abs(m_filterFlag) % 1000 - getPrefilterFlag()) / 10;
    }

    /**
     * Get the flag showing if the distance at kink vertex criteria was failed for split track.
     * For details, refer to the documentation of Kink::m_filterFlag.
     */
    short getSplitTrackDistanceAtVertexFlag() const
    {
      return getCombinedFitResultFlag();
    }

    /**
     * Get signed number of reassigned hits (- from daughter to mother, + from mother to daughter).
     * For details, refer to the documentation of Kink::m_filterFlag.
     */
    short getNumberOfReassignedHits() const
    {
      return m_filterFlag / 1000;
    }

  private:
    /** Indicates which mother `Track` was used for this `Kink`. */
    short m_trackIndexMother = -1;

    /** Indicates which daughter `Track` was used for this `Kink` (it is the same as mother's in case of track splitting). */
    short m_trackIndexDaughter = -1;

    /** Points to the new `TrackFitResult` of the mother `Track` at Start. */
    short m_trackFitResultIndexMotherStart = -1;

    /** Points to the new `TrackFitResult` of the mother `Track` at End. */
    short m_trackFitResultIndexMotherEnd = -1;

    /** Points to the new `TrackFitResult` of the daughter `Track` at Start. */
    short m_trackFitResultIndexDaughter = -1;

    /** The X coordinate of the fitted kink vertex. */
    Double32_t m_fittedVertexX = 0.0;

    /** The Y coordinate of the fitted kink vertex. */
    Double32_t m_fittedVertexY = 0.0;

    /** The Z coordinate of the fitted kink vertex. */
    Double32_t m_fittedVertexZ = 0.0;

    /** The filter flag of the kink.
     *
     * 1st digit represents the filter in `KinkFinderModule` used to preselect the kink candidate (from 1 to 5):
     * Filter 1, 2, 4, and 5 of `KinkFinderModule` are saved as 1 (track pair that has close endpoints);
     * Filter 3 and 6 of `KinkFinderModule` are saved as 2 (track pair selected with daughter Helix extrapolation);
     * Filter 7, 8, and 9 of `KinkFinderModule` are saved as 3, 4, and 5, respectively (split tracks).
     * Split-track kinks (3-5) have worse resolutions compared to track-pair kinks (1-2).
     * The filter number (3-5) for split-track kinks might be helpful to suppress false track splitting with ML.
     * The kinks created from track pair that has close endpoints (1) might have better resolutions than ones selected
     * with daughter Helix extrapolation (2).
     *
     * 2nd and 3rd digits for filter 1-6 of `KinkFinderModule` represent the result of combined fit (from 0 to 15, 18 and 19);
     * 19 means fit of combined track is failed; 18 means combined track has less n.d.f. than mother track;
     * if <16, it should be understood in bit form, where the results of following comparisons are 1 or 0:
     * 1st bit: p-value of combined fit > p-value of mother fit; 2nd bit: p-value of combined fit > p-value of daughter fit;
     * 3rd bit: n.d.f. of combined track > n.d.f. daughter track; 4th bit: p-value of combined fit > 10^{−7}.
     *
     * 2nd digit for filter 7-9 of `KinkFinderModule` shows 1 (0) if the distance at the fitted vertex is bigger (smaller)
     * than required cut in `KinkFitter`.
     *
     * 4th and 5th digits together form a number of reassigned hits (from 0 to 32);
     * If more than 32 hits are reassigned, set 32.
     *
     * The sign shows from which track the hits were taken (- from daughter, + from mother).
     *
     * The content of the m_filterFlag may change, please, refer for the details to `KinkFitter.cc`.
     *
     * Example 1: -23181: -2318"1" track pair was preselected with filter 1, 2, 4, or 5
     * (mother track and daughter track end points are close); -23"18"1 the exit code of combined fit is 18, so
     * the combined track has less n.d.f. than mother track; "-23"181 -- 23 hits were reassigned from daughter track
     * to mother track
     * Example 2: 2015: 201"5" kink was created from split track selected with filter 9 (track did not pass
     * neither mother nor daughter preselection criteria); 2"01"5 the code 1 means that the distance at the fitted
     * kink vertex is larger than the set cut; "2"015 -- 2 hits were reassigned from mother track to daughter track
     */
    short m_filterFlag = 0;

    /** Macro for ROOTification. */
    ClassDef(Kink, 1);

    friend class FixMergedObjectsModule;
  };
}
