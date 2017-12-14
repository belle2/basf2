/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>

#include <tracking/trackFindingCDC/topology/ISuperLayer.h>

#include <tracking/trackFindingCDC/ca/AutomatonCell.h>

#include <tracking/trackFindingCDC/ca/Path.h>

#include <tracking/trackFindingCDC/utilities/MayBePtr.h>

#include <vector>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCSegment2D;
    class CDCSegment3D;
    class CDCSegmentPair;
    class CDCSegmentTriple;

    class CDCWireHit;
    class Vector2D;
    class Vector3D;

    /// Class representing a sequence of three dimensional reconstructed hits
    class CDCTrack : public std::vector<CDCRecoHit3D> {
    public:

      /// Default constructor for ROOT compatibility.
      CDCTrack() = default;

      /// Constructor from a series of hits.
      explicit CDCTrack(const std::vector<CDCRecoHit3D>& recoHits3D);

      /// Constructor from a two dimensional segment filling the third dimension with 0 values.
      explicit CDCTrack(const CDCSegment2D& segment);

      /// Concats several tracks from a path
      static CDCTrack condense(const Path<const CDCTrack>& trackPath);

      /// Reconstructs the hit content of the segment triple track to a CDCTrack averaging overlapping parts
      static CDCTrack condense(const Path<const CDCSegmentTriple>& segmentTriplePath);

      /// Reconstructs the hit content of the axial stereo segment pair path to a CDCTrack averaging overlapping parts.
      static CDCTrack condense(const Path<const CDCSegmentPair>& segmentPairPath);

      /// Comparision of track - no particular order has been defined so far, all tracks are equivalent
      bool operator<(const CDCTrack& track __attribute__((unused))) const
      {
        return false;
      }

      /** Splits the track into segments.
       *  Note : No trajectory information is copied*/
      std::vector<CDCSegment3D> splitIntoSegments() const;

      /// Getter for the superlayer id the track starts from.
      ISuperLayer getStartISuperLayer() const
      {
        return front().getISuperLayer();
      }

      /// Getter for the superlayer id the track ends in.
      ISuperLayer getEndISuperLayer() const
      {
        return back().getISuperLayer();
      }

      /// Getter for the position of the first reconstructed hit.
      const Vector3D& getStartRecoPos3D() const
      {
        return front().getRecoPos3D();
      }

      /// Getter for the position of the last reconstructed hit.
      const Vector3D& getEndRecoPos3D() const
      {
        return back().getRecoPos3D();
      }

      /// Setter for the two dimensional trajectory. The trajectory should start at the start of the
      /// track and follow its direction.
      void setStartTrajectory3D(const CDCTrajectory3D& startTrajectory3D)
      {
        m_startTrajectory3D = startTrajectory3D;
      }

      /// Setter for the three dimensional trajectory. The trajectory should start at the END of the
      /// track and *follow* its direction.
      void setEndTrajectory3D(const CDCTrajectory3D& endTrajectory3D)
      {
        m_endTrajectory3D = endTrajectory3D;
      }

      /// Getter for the two dimensional trajectory. The trajectory should start at the start of the
      /// track and follow its direction.
      const CDCTrajectory3D& getStartTrajectory3D() const
      {
        return m_startTrajectory3D;
      }

      /// Getter for the three dimensional trajectory. The trajectory should start at the END of the
      /// track and follow its direction.
      const CDCTrajectory3D& getEndTrajectory3D() const
      {
        return m_endTrajectory3D;
      }

      /// Mutable getter for the automaton cell.
      AutomatonCell& getAutomatonCell() const
      {
        return m_automatonCell;
      }

      /// Indirection to the automaton cell for easier access to the flags
      AutomatonCell* operator->() const
      {
        return &m_automatonCell;
      }

      /// Unset the masked flag of the automaton cell of this segment and of all contained wire hits.
      void unsetAndForwardMaskedFlag() const;

      /** Set the masked flag of the automaton cell of this segment and
       *  forward the masked flag to all contained wire hits.
       */
      void setAndForwardMaskedFlag() const;

      /** Check all contained wire hits if one has the masked flag.
       *  Set the masked flag of this segment in case at least one
       * of the contained wire hits is flagged as masked.
       */
      void receiveMaskedFlag() const;

      /** Set the taken flag of all hits belonging to this track to the given value (default true),
       * but do not touch the flag of the track itself. */
      void forwardTakenFlag(bool takenFlag = true) const;


      /// Sort the recoHits according to their perpS information
      void sortByArcLength2D();

      /// Set all arcLengths to have positive values by shifting them by pi*radius if they are negative.
      /// This can only be done if the radius is not infinity (for example cosmics).
      /// The flag can be used to do this for all tracks (default is to do this only for curlers)
      void shiftToPositiveArcLengths2D(bool doForAllTracks = false);

      /// Reverse the track inplace
      void reverse();

      /// Return a reversed copy of the track.
      CDCTrack reversed() const;

      /// Finds the first CDCRecoHit3D that is based on the given wire hit - nullptr if none
      MayBePtr<const CDCRecoHit3D> find(const CDCWireHit& wireHit) const;

      /// Set the flag which indicates that the track has a matching segment (probably only used in the SegmentTrackCombiner).
      void setHasMatchingSegment(bool hasMatchingSegment = true)
      {
        m_hasMatchingSegment = hasMatchingSegment;
      }

      /**
       * Get a flag which indicates that the track has a matching segment (probably set in the SegmentTrackCombiner).
       * This flag can be used for filter decisions (e.g. if the track is fake).
       */
      bool getHasMatchingSegment() const
      {
        return m_hasMatchingSegment;
      }

    private:
      /// Memory for the automaton cell.
      mutable AutomatonCell m_automatonCell;

      /// Memory for the three dimensional trajectory at the start of the track
      CDCTrajectory3D m_startTrajectory3D;

      /// Memory for the three dimensional trajectory at the end of the track
      CDCTrajectory3D m_endTrajectory3D;

      /// Flag which indicates that the track had a matching segment (can be used for filter decisions)
      bool m_hasMatchingSegment = false;
    };
  }
}
