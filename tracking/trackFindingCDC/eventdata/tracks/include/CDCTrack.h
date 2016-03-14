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

#include <tracking/trackFindingCDC/ca/Path.h>

#include <vector>

namespace genfit {
  class TrackCand;
}

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCRecoSegment2D;
    class CDCRecoSegment3D;
    class CDCSegmentPair;
    class CDCSegmentTriple;

    /// Class representing a sequence of three dimensional reconstructed hits
    class CDCTrack : public std::vector<Belle2::TrackFindingCDC::CDCRecoHit3D> {
    public:

      /// Default constructor for ROOT compatibility.
      CDCTrack() {}

      /// Constructor from a two dimensional segment filling the thrid dimension with 0 values.
      explicit CDCTrack(const CDCRecoSegment2D& segment);

      /// Concats several tracks from a path
      static CDCTrack condense(const Path<const CDCTrack>& trackPath);

      /// Reconstructs the hit content of the segment triple track to a CDCTrack averaging overlapping parts
      static CDCTrack condense(const Path<const CDCSegmentTriple>& segmentTriplePath);

      /// Reconstructs the hit content of the axial stereo segment pair path to a CDCTrack averaging overlapping parts.
      static CDCTrack condense(const Path<const CDCSegmentPair>& segmentPairPath);

      /// Copies the hit content of the hit vector track to the CDCTrack. Do not use any taken or masked hits.
      void appendNotTaken(const std::vector<const CDCWireHit*>& hits);

      /// Copies the hit and trajectory content of this track to the Genfit track candidate
      bool fillInto(genfit::TrackCand& trackCand) const;

      /** Splits the track into segments.
       *  Note : No trajectory information is copied*/
      std::vector<Belle2::TrackFindingCDC::CDCRecoSegment3D>
      splitIntoSegments() const;

      /// Getter for the first reconstructed hit in the track.
      const Belle2::TrackFindingCDC::CDCRecoHit3D& getStartRecoHit3D() const
      { return front(); }

      /// Getter for the last reconstructed hit in the track.
      const Belle2::TrackFindingCDC::CDCRecoHit3D& getEndRecoHit3D() const
      { return back(); }



      /// Getter for the superlayer id the track starts from.
      ISuperLayer getStartISuperLayer() const
      { return getStartRecoHit3D().getISuperLayer(); }

      /// Getter for the superlayer id the track ends in.
      ISuperLayer getEndISuperLayer() const
      { return getEndRecoHit3D().getISuperLayer(); }



      /// Getter for the position of the first reconstructed hit.
      const Vector3D& getStartRecoPos3D() const
      { return getStartRecoHit3D().getRecoPos3D(); }

      /// Getter for the position of the last reconstructed hit.
      const Vector3D& getEndRecoPos3D() const
      { return getEndRecoHit3D().getRecoPos3D(); }



      /// Getter for the start fitted position of track.
      Vector3D getStartFitPos3D() const
      { return getStartTrajectory3D().getSupport(); }

      /// Getter for the end fitted position of track.
      Vector3D getEndFitPos3D() const
      { return getEndTrajectory3D().getSupport(); }


      /// Getter for the momentum at the start position.
      Vector3D getStartFitMom3D() const
      { return getStartTrajectory3D().getUnitMom3DAtSupport(); }

      /// Getter for the momentum at the end position.
      Vector3D getEndFitMom3D() const
      { return getEndTrajectory3D().getUnitMom3DAtSupport(); }

      /// Getter for the charge sign.
      ESign getStartChargeSign() const
      { return getStartTrajectory3D().getChargeSign(); }

      /// Getter for the charge sign.
      ESign getEndChargeSign() const
      { return getEndTrajectory3D().getChargeSign(); }

      /// Setter for the two dimensional trajectory. The trajectory should start at the start of the track and follow its direction.
      void setStartTrajectory3D(const CDCTrajectory3D& startTrajectory3D)
      { m_startTrajectory3D = startTrajectory3D; }

      /// Setter for the three dimensional trajectory. The trajectory should start at the END of the track and *follow* its direction.
      void setEndTrajectory3D(const CDCTrajectory3D& endTrajectory3D)
      { m_endTrajectory3D = endTrajectory3D; }

      /// Getter for the two dimensional trajectory. The trajectory should start at the start of the track and follow its direction.
      const CDCTrajectory3D& getStartTrajectory3D() const
      { return m_startTrajectory3D; }

      /// Getter for the three dimensional trajectory. The trajectory should start at the END of the track and follow its direction.
      const CDCTrajectory3D& getEndTrajectory3D() const
      { return m_endTrajectory3D; }

      /// Getter for the automaton cell.
      AutomatonCell& getAutomatonCell()
      { return m_automatonCell; }

      /// Constant getter for the automaton cell.
      const AutomatonCell& getAutomatonCell() const
      { return m_automatonCell; }

      /// Unset the masked flag of the automaton cell of this segment and of all contained wire hits.
      void unsetAndForwardMaskedFlag() const;

      /** Set the masked flag of the automaton cell of this segment and
       *  forward the masked flag to all contained wire hits.
       */
      void setAndForwardMaskedFlag() const;

      /** Check all contained wire hits if one has the masked flag.
       *  Set the masked flag of this segment in case at least one
       of the contained wire hits is flagged as masked.
      */
      void receiveMaskedFlag() const;

      /** Remove all hits from the track which have an assigned flag */
      void removeAllAssignedMarkedHits();

      /** Set the taken flag of all hits belonging to this track to the given value (default true),
       * but do not touch the flag of the track itself. */
      void forwardTakenFlag(bool takenFlag = true) const;

      /// Sort the recoHits according to their perpS information
      void sortByArcLength2D()
      {
        std::stable_sort(begin(), end(),
        [](const CDCRecoHit3D & recoHit, const CDCRecoHit3D & otherRecoHit) {
          return recoHit.getArcLength2D() < otherRecoHit.getArcLength2D();
        });
      }

      /// Set all arcLengths to have positive values by shifting them by pi*radius if they are negative.
      /// This can only be done if the radius is not infinity (for example cosmics).
      /// The flag can be used to do this for all tracks (default is to do this only for curlers)
      void shiftToPositiveArcLengths2D(bool doForAllTracks = false);

      /// Sort the CDCRecoHits by their CDCWireHit.
      void sortByRadius();

      /// Legacy accessor for the CDCRecoHit3D of the track, still used in some corners of python scripts.
      const std::vector<CDCRecoHit3D>& items() const
      { return *this; }

      /// Reverse the track inplace
      void reverse();

      /// Return a reversed copy of the track.
      CDCTrack reversed() const;

      /// Return the pointer to the track candidate which was created from this CDCrack
      genfit::TrackCand* getRelatedGenfitTrackCandidate() const
      {
        return m_relatedGenfitTrackCandidate;
      }

      /// Set the pointer to the track candidate which was created from this CDCrack
      void setRelatedGenfitTrackCandidate(genfit::TrackCand* relatedTrackCand)
      {
        m_relatedGenfitTrackCandidate = relatedTrackCand;
      }

      void setHasMatchingSegment(bool hasMatchingSegment = true)
      {
        m_hasMatchingSegment = hasMatchingSegment;
      }

      bool getHasMatchingSegment() const
      {
        return m_hasMatchingSegment;
      }

    private:
      /// Memory for the automaton cell.
      AutomatonCell m_automatonCell;

      /// Memory for the three dimensional trajectory at the start of the track
      CDCTrajectory3D m_startTrajectory3D;

      /// Memory for the three dimensional trajectory at the end of the track
      CDCTrajectory3D m_endTrajectory3D;

      /// Related genfit track cand
      genfit::TrackCand* m_relatedGenfitTrackCandidate = nullptr;

      /// Flag which indicates that the track had a matching segment (can be used for filter decisions)
      bool m_hasMatchingSegment = false;

    }; //class

  } // namespace TrackFindingCDC
} // namespace Belle2
