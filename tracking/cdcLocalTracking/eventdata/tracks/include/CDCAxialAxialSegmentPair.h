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

#include <tracking/cdcLocalTracking/rootification/SwitchableRootificationBase.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/algorithms/AutomatonCell.h>

#include <tracking/cdcLocalTracking/eventdata/entities/CDCEntities.h>
#include <tracking/cdcLocalTracking/eventdata/segments/CDCSegments.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class representing a pair of reconstructed axial segements in adjacent superlayer
    class CDCAxialAxialSegmentPair : public SwitchableRootificationBase {
    public:

      /// Default constructor - for ROOT compatability
      CDCAxialAxialSegmentPair();

      /// Constructor from two segments
      CDCAxialAxialSegmentPair(const CDCAxialRecoSegment2D* startSegment, const CDCAxialRecoSegment2D* endSegment);

      /// Constructor from two segments and an assoziated trajectory
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

      /// Total ordering sheme comparing the segment pointers.
      bool operator<(CDCAxialAxialSegmentPair const& rhs) const {
        return (getStart() < rhs.getStart()  or
                (getStart() == rhs.getStart() and getEnd() < rhs.getEnd()));
      }

      /// Define reconstructed segments and segment triples as coaligned on the start segment
      friend bool operator<(CDCAxialAxialSegmentPair const& segmentTriple , const CDCAxialRecoSegment2D* axialSegment)
      { return segmentTriple.getStart() < axialSegment; }

      /// Define reconstructed segments and segment triples as coaligned on the start segment
      friend bool operator<(const CDCAxialRecoSegment2D* axialSegment, CDCAxialAxialSegmentPair const& segmentTriple)
      { return axialSegment < segmentTriple.getStart(); }

      /// Allow automatic taking of the address.
      /** Essentially pointers to (lvalue) objects is a subclass of the object itself.
       *  This method activally exposes this inheritance to be able to write algorithms that work for objects and poiinters alike without code duplication.
       *  \note Once reference qualifiers become available use an & after the trailing const to constrain the cast to lvalues.*/
      operator const Belle2::CDCLocalTracking::CDCAxialAxialSegmentPair* () const
      { return this; }



      /// Checks the references to the contained three segment for nullptrs.
      bool checkSegments() const
      { return not(m_startSegment == nullptr or m_endSegment == nullptr); }


      /// Getter for the superlayer id of the start segment.
      ILayerType getStartISuperLayer() const
      { return getStart() == nullptr ? INVALID_ISUPERLAYER : getStart()->getISuperLayer(); }

      /// Getter for the superlayer id of the end segment.
      ILayerType getEndISuperLayer() const
      { return getEnd() == nullptr ? INVALID_ISUPERLAYER : getEnd()->getISuperLayer(); }

      /// Getter for the start segment.
      const CDCAxialRecoSegment2D* getStart() const
      { return m_startSegment; }

      /// Setter for the start segment.
      void setStart(const CDCAxialRecoSegment2D* startSegment)
      { m_startSegment = startSegment; }

      /// Getter for the end segment.
      const CDCAxialRecoSegment2D* getEnd() const
      { return m_endSegment; }

      /// Setter for the end segment.
      void setEnd(const CDCAxialRecoSegment2D* endSegment)
      { m_endSegment = endSegment; }


      /// Getter for the trajectory of the two dimensional trajectory
      CDCTrajectory2D& getTrajectory2D() const
      { return m_trajectory2D; }

      /// Setter for the trajectory of the two dimensional trajectory
      void setTrajectory2D(const CDCTrajectory2D& trajectory2D) const
      { m_trajectory2D =  trajectory2D; }

      /// Invalides the currently stored trajectory information
      void clearTrajectory2D() const
      { getTrajectory2D().clear(); }

      /// Sets the do not use flag of the segment pair's automaton cell. Also forward the don not use flag to the contained segments and the contained wire hits.
      void setAndForwardDoNotUseFlag() const {
        getAutomatonCell().setDoNotUseFlag();
        getStart()->setAndForwardDoNotUseFlag();
        getEnd()->setAndForwardDoNotUseFlag();
      }

      /// If one of the contained segments is marked as do not use this segment triple is set be not usable as well
      void receiveDoNotUseFlag() const {
        getStart()->receiveDoNotUseFlag();
        getEnd()->receiveDoNotUseFlag();

        if (getStart()->getAutomatonCell().hasDoNotUseFlag() or
            getEnd()->getAutomatonCell().hasDoNotUseFlag()) {
          getAutomatonCell().setDoNotUseFlag();
        }

      }

      /// Checks if the last entity in the vector lies greater or lower travel distance than the last entity.
      /** Returns:
       *  * FORWARD if the last entity lies behind the first.
       *  * BACKWARD if the last entity lies before the first.
       */
      ForwardBackwardInfo isCoaligned(const CDCTrajectory2D& trajectory2D) const {
        ForwardBackwardInfo startIsCoaligned = getStart()->isCoaligned(trajectory2D);
        ForwardBackwardInfo endIsCoaligned = getEnd()->isCoaligned(trajectory2D);

        if (startIsCoaligned == FORWARD and endIsCoaligned == FORWARD) return FORWARD;
        else if (startIsCoaligned == BACKWARD and endIsCoaligned == BACKWARD) return BACKWARD;
        else return INVALID_INFO;
      }

      /// Getter for the automaton cell.
      AutomatonCell& getAutomatonCell() { return m_automatonCell; }

      /// Constant getter for the automaton cell.
      const AutomatonCell& getAutomatonCell() const { return m_automatonCell; }

    private:

      const CDCAxialRecoSegment2D* m_startSegment; ///< Reference to the start segment
      const CDCAxialRecoSegment2D* m_endSegment; ///< Reference to the end segment

      mutable CDCTrajectory2D m_trajectory2D; ///< Reference to the common trajectory
      mutable AutomatonCell m_automatonCell; ///< Automaton cell assoziated with the pair of segments

      /** ROOT Macro to make CDCAxialAxialSegmentPair a ROOT class.*/
      CDCLOCALTRACKING_SwitchableClassDef(CDCAxialAxialSegmentPair, 1);

    }; //end class CDCAxialAxialSegmentPair

  } // end namespace CDCLocalTracking
} // end namespace Belle2
#endif // CDCAXIALAXIALSEGMENTPAIR
