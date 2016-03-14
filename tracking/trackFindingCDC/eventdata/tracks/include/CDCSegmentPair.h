/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/BasicTypes.h>
#include <tracking/trackFindingCDC/ca/AutomatonCell.h>

#include <tracking/trackFindingCDC/eventdata/entities/CDCEntities.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegments.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class representing a pair of one reconstructed axial segement and one stereo segment in adjacent superlayer.
    class CDCSegmentPair  {
    public:

      /// Default constructor - for ROOT compatability.
      CDCSegmentPair();

      /// Constructor from two segments
      CDCSegmentPair(const CDCRecoSegment2D* startSegment,
                     const CDCRecoSegment2D* endSegment);

      /// Constructor from two segments and an assoziated trajectory
      CDCSegmentPair(const CDCRecoSegment2D* startSegment,
                     const CDCRecoSegment2D* endSegment,
                     const CDCTrajectory3D& trajectory3D);

      /// Empty destructor
      ~CDCSegmentPair();

      /// Equality comparision based on the pointers to the stored segments.
      bool operator==(CDCSegmentPair const& rhs) const
      { return getStartSegment() == rhs.getStartSegment() and getEndSegment() == rhs.getEndSegment(); }



      /// Total ordering sheme comparing the segment pointers.
      bool operator<(CDCSegmentPair const& rhs) const
      {
        return (getStartSegment() < rhs.getStartSegment()  or
                (getStartSegment() == rhs.getStartSegment() and getEndSegment() < rhs.getEndSegment()));
      }

      /// Define reconstructed segments and axial stereo segment pairs as coaligned on the start segment
      friend bool operator<(const CDCSegmentPair& segmentPair,
                            const CDCRecoSegment2D* segment)
      { return segmentPair.getStartSegment() < segment; }

      /// Define reconstructed segments and axial stereo segment pairs as coaligned on the start segment
      friend bool operator<(const CDCRecoSegment2D* segment,
                            const CDCSegmentPair& segmentPair)
      { return segment < segmentPair.getStartSegment(); }



      /// Allow automatic taking of the address.
      /** Essentially pointers to (lvalue) objects is a subclass of the object itself.
       *  This method activally exposes this inheritance to be able to write algorithms
       *  that work for objects and poiinters alike without code duplication.
       *  \note Once reference qualifiers become available use an & after the trailing const
       *  to constrain the cast to lvalues.*/
      operator const Belle2::TrackFindingCDC::CDCSegmentPair* () const
      { return this; }



      /// Checks if both stored segments are not nullptr. Returns true if check is succeded.
      bool checkSegmentsNonNullptr() const
      { return getStartSegment() != nullptr and getEndSegment() != nullptr; }


      /// Checks if the two segments are of different axial type.
      bool checkSegmentsStereoTypes() const
      { return getStartStereoType() xor getEndStereoType(); }

      /// Checks the references to the contained three segment for nullptrs and exactly one of them is axial and one is stereo
      bool checkSegments() const
      { return checkSegmentsNonNullptr() and checkSegmentsStereoTypes(); }



      /// Getter for the stereo type of the first segment.
      StereoType getStartStereoType() const
      { return getStartSegment() == nullptr ? INVALID_STEREOTYPE : getStartSegment()->getStereoType(); }


      /// Getter for the stereo type of the second segment.
      StereoType getEndStereoType() const
      { return getEndSegment() == nullptr ? INVALID_STEREOTYPE : getEndSegment()->getStereoType(); }



      /// Getter for the superlayer id of the start segment.
      ILayerType getStartISuperLayer() const
      { return getStartSegment() == nullptr ? INVALID_ISUPERLAYER : getStartSegment()->getISuperLayer(); }

      /// Getter for the superlayer id of the end segment.
      ILayerType getEndISuperLayer() const
      { return getEndSegment() == nullptr ? INVALID_ISUPERLAYER : getEndSegment()->getISuperLayer(); }



      /// Getter for the start segment.
      const CDCRecoSegment2D* getStartSegment() const
      { return m_startSegment; }

      /// Setter for the start segment.
      void setStartSegment(const CDCRecoSegment2D* startSegment)
      { setSegments(startSegment, getEndSegment()); }



      /// Getter for the end segment.
      const CDCRecoSegment2D* getEndSegment() const
      { return m_endSegment; }

      /// Setter for the end segment.
      void setEndSegment(const CDCRecoSegment2D* endSegment)
      { setSegments(getStartSegment(), endSegment); }



      /// Getter for the stereo segment
      const CDCAxialRecoSegment2D* getStereoSegment() const
      { return getStartStereoType() ? getStartSegment() : getEndSegment(); }

      /// Getter for the axial segment
      const CDCAxialRecoSegment2D* getAxialSegment() const
      { return getStartStereoType() ? getEndSegment() : getStartSegment(); }


      /// Setter for both segments simultaniously
      void setSegments(const CDCRecoSegment2D* startSegment, const CDCRecoSegment2D* endSegment)
      {
        m_startSegment = startSegment;
        m_endSegment = endSegment;
        if (not checkSegmentsNonNullptr()) B2WARNING("CDCSegmentPair: one segment set to nullptr");
        if (not checkSegmentsStereoTypes()) B2WARNING("CDCSegmentPair: segments set with same stereo type");
      }

      /** Determines the angle between the last reconstructed position of the start segment and
       *  the first reconstructed position of the end segment.*/
      FloatType computeDeltaPhiAtSuperLayerBound() const;

      /** Indicator if the start segment lies before the end segment */
      FloatType computeStartIsBeforeEnd() const
      { return computeStartIsBeforeEndFitless(); }

      /** Indicator if the start segment lies before the end segment */
      FloatType computeEndIsAfterStart() const
      { return computeEndIsAfterStartFitless(); }

      /** Indicator if the start segment and the end segment have roughly the same travel direction.*/
      FloatType computeIsCoaligned() const
      { return computeIsCoalignedFitless(); }

      /** Indicator if the start segment lies before the end segment, build without using the trajectories, which may not have been fitted yet. */
      FloatType computeStartIsBeforeEndFitless() const;

      /** Indicator if the start segment lies before the end segment, build without using the trajectories, which may not have been fitted yet. */
      FloatType computeEndIsAfterStartFitless() const;

      /** Indicator if the start segment and the end segment have roughly the same travel direction without using the common fit*/
      FloatType computeIsCoalignedFitless() const;

      /// Checks if the last entity in the vector lies greater or lower travel distance than the last entity.
      /** Returns:
       *  * FORWARD if the last entity lies behind the first.
       *  * BACKWARD if the last entity lies before the first.
       */
      ForwardBackwardInfo isCoaligned(const CDCTrajectory2D& trajectory2D) const
      {
        ForwardBackwardInfo startIsCoaligned = trajectory2D.isForwardOrBackwardTo(*(getStartSegment()));
        ForwardBackwardInfo endIsCoaligned = trajectory2D.isForwardOrBackwardTo(*(getEndSegment()));

        if (startIsCoaligned == FORWARD and endIsCoaligned == FORWARD) return FORWARD;
        else if (startIsCoaligned == BACKWARD and endIsCoaligned == BACKWARD) return BACKWARD;
        else return INVALID_INFO;
      }

      /// Setter for the three dimensional trajectory
      void setTrajectory3D(const CDCTrajectory3D& trajectory3D) const
      { m_trajectory3D = trajectory3D; }

      /// Getter for the three dimensional trajectory.
      CDCTrajectory3D& getTrajectory3D() const
      { return m_trajectory3D; }

      /// Getter for the two dimensional projection of the common three dimensional trajectory.
      const CDCTrajectory2D getTrajectory2D() const
      { return getTrajectory3D().getTrajectory2D(); }

      /// Getter for the sz projection of the common three dimensional trajectory.
      const CDCTrajectorySZ getTrajectorySZ() const
      { return getTrajectory3D().getTrajectorySZ(); }

      /// Invalides the currently stored trajectory information.
      void clearTrajectory3D() const
      { getTrajectory3D().clear(); }


      /// Unsets the masked flag of the segment triple's automaton cell and of the three contained segments.
      void unsetAndForwardMaskedFlag() const
      {
        getAutomatonCell().unsetMaskedFlag();
        getStartSegment()->unsetAndForwardMaskedFlag();
        getEndSegment()->unsetAndForwardMaskedFlag();
      }

      /// Sets the masked flag of the segment triple's automaton cell and of the three contained segments.
      void setAndForwardMaskedFlag() const
      {
        getAutomatonCell().setMaskedFlag();
        getStartSegment()->setAndForwardMaskedFlag();
        getEndSegment()->setAndForwardMaskedFlag();
      }

      /// If one of the contained segments is marked as masked this segment triple is set be masked as well.
      void receiveMaskedFlag() const
      {
        getStartSegment()->receiveMaskedFlag();
        getEndSegment()->receiveMaskedFlag();

        if (getStartSegment()->getAutomatonCell().hasMaskedFlag() or
            getEndSegment()->getAutomatonCell().hasMaskedFlag()) {
          getAutomatonCell().setMaskedFlag();
        }
      }

      /// Getter for the automaton cell.
      AutomatonCell& getAutomatonCell()
      { return m_automatonCell; }

      /// Constant getter for the automaton cell.
      const AutomatonCell& getAutomatonCell() const
      { return m_automatonCell; }



    private:
      const CDCRecoSegment2D* m_startSegment; ///< Reference to the start segment
      const CDCRecoSegment2D* m_endSegment; ///< Reference to the end segment

      /// Memory for the common three dimensional trajectory
      mutable CDCTrajectory3D m_trajectory3D;

      mutable AutomatonCell m_automatonCell; ///< Automaton cell assoziated with the pair of segments


    }; //end class CDCSegmentPair

  } // end namespace TrackFindingCDC
} // end namespace Belle2
