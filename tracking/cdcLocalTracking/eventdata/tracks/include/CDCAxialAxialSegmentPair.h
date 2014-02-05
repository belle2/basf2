/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCAXIALAXIALSEGMENTPAIR_H
#define CDCAXIALAXIALSEGMENTPAIR_H

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/algorithms/AutomatonCell.h>

#include <tracking/cdcLocalTracking/eventdata/entities/CDCEntities.h>
#include <tracking/cdcLocalTracking/eventdata/segments/CDCSegments.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class representing a pair of reconstructed axial segements in adjacent superlayer
    class CDCAxialAxialSegmentPair : public UsedTObject {
    public:

      /** Constructor. */
      /** This is as well the parameter free I/O constructor.
       */

      CDCAxialAxialSegmentPair();

      CDCAxialAxialSegmentPair(const CDCAxialRecoSegment2D* startSegment, const CDCAxialRecoSegment2D* endSegment);

      CDCAxialAxialSegmentPair(
        const CDCAxialRecoSegment2D* startSegment,
        const CDCAxialRecoSegment2D* endSegment,
        const CDCTrajectory2D& trajectory2D
      );

      /// Empty destructor
      ~CDCAxialAxialSegmentPair();

      /// Equality comparision based on the pointers to the stored segments.
      bool operator==(CDCAxialAxialSegmentPair const& rhs) const
      { return getStart() == rhs.getStart() and getEnd() == rhs.getEnd(); }

      bool operator<(CDCAxialAxialSegmentPair const& rhs) const {
        return (getStart() < rhs.getStart()  or
                (getStart() == rhs.getStart() and   getEnd() < rhs.getEnd()));
      }

      /// Define reconstructed segments and segment triples as coaligned on the start segment
      friend bool operator<(CDCAxialAxialSegmentPair const& segmentTriple , const CDCAxialRecoSegment2D* axialSegment)
      { return segmentTriple.getStart() < axialSegment; }

      /// Define reconstructed segments and segment triples as coaligned on the start segment
      friend bool operator<(const CDCAxialRecoSegment2D* axialSegment, CDCAxialAxialSegmentPair const& segmentTriple)
      { return axialSegment < segmentTriple.getStart(); }

      const CDCAxialAxialSegmentPair* operator->() const { return this; }

      /// Checks the references to the contained three segment for nullptrs
      bool checkSegments() const
      { return not(m_startSegment == nullptr or m_endSegment == nullptr); }


      ILayerType getStartISuperLayer() const
      { return getStart() == nullptr ? INVALIDSUPERLAYER : getStart()->getISuperLayer(); }

      ILayerType getEndISuperLayer() const
      { return getEnd() == nullptr ? INVALIDSUPERLAYER : getEnd()->getISuperLayer(); }

      const CDCAxialRecoSegment2D*   getStart()   const { return m_startSegment; }
      void setStart(const CDCAxialRecoSegment2D* startSegment) { m_startSegment = startSegment; }

      const CDCAxialRecoSegment2D*   getEnd()     const { return m_endSegment; }
      void setEnd(const CDCAxialRecoSegment2D* endSegment) { m_endSegment = endSegment; }

      CDCTrajectory2D& getTrajectory2D() const
      { return m_trajectory2D; }

      void setTrajectory2D(const CDCTrajectory2D& trajectory2D) const
      { m_trajectory2D =  trajectory2D; }

      /// Sets the do not use flag of the segment triple's automaton cell and of the three contained segments
      void setDoNotUse() const {
        getAutomatonCell().setFlags(DO_NOT_USE);
        forwardDoNotUse();
      }

      /// Sets the do not use flag of the three contained segments
      void forwardDoNotUse() const {
        getStart()->getAutomatonCell().setFlags(DO_NOT_USE);
        getEnd()->getAutomatonCell().setFlags(DO_NOT_USE);
      }

      /// If one of the contained segments is marked as do not use this segment triple is set be not usable as well
      void receiveDoNotUse() const {
        if (getStart()->getAutomatonCell().hasAnyFlags(DO_NOT_USE) or
            getEnd()->getAutomatonCell().hasAnyFlags(DO_NOT_USE)) {

          getAutomatonCell().setFlags(DO_NOT_USE);
        }

      }

      /// Getter for the automaton cell.
      AutomatonCell& getAutomatonCell() { return m_automatonCell; }

      /// Constant getter for the automaton cell.
      const AutomatonCell& getAutomatonCell() const { return m_automatonCell; }

    private:

      const CDCAxialRecoSegment2D* m_startSegment;
      const CDCAxialRecoSegment2D* m_endSegment;

      mutable CDCTrajectory2D m_trajectory2D;
      mutable AutomatonCell m_automatonCell;

      /** ROOT Macro to make CDCAxialAxialSegmentPair a ROOT class.*/
      ClassDefInCDCLocalTracking(CDCAxialAxialSegmentPair, 1);

    }; //end class CDCAxialAxialSegmentPair

  } // end namespace CDCLocalTracking
} // end namespace Belle2
#endif // CDCAXIALAXIALSEGMENTPAIR
