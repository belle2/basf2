/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCSEGMENTTRIPLE_H
#define CDCSEGMENTTRIPLE_H

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/algorithms/AutomatonCell.h>

#include <tracking/cdcLocalTracking/eventdata/entities/CDCEntities.h>
#include <tracking/cdcLocalTracking/eventdata/segments/CDCSegments.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class representing a triple of reconstructed segements in adjacent superlayer
    class CDCSegmentTriple : public UsedTObject {
    public:

      /** Constructor. */
      /** This is as well the parameter free I/O constructor.
       */

      CDCSegmentTriple() :
        m_startSegment(nullptr),
        m_middleSegment(nullptr),
        m_endSegment(nullptr)
      {;}

      CDCSegmentTriple(const CDCAxialRecoSegment2D* startSegment , const CDCAxialRecoSegment2D* endSegment) :
        m_startSegment(startSegment),
        m_middleSegment(nullptr),
        m_endSegment(endSegment)
      {;}

      CDCSegmentTriple(
        const CDCAxialRecoSegment2D* startSegment,
        const CDCStereoRecoSegment2D* middleSegment,
        const CDCAxialRecoSegment2D* endSegment
      ) :
        m_startSegment(startSegment),
        m_middleSegment(middleSegment),
        m_endSegment(endSegment)
      {;}


      CDCSegmentTriple(
        const CDCAxialRecoSegment2D* startSegment,
        const CDCStereoRecoSegment2D* middleSegment,
        const CDCAxialRecoSegment2D* endSegment,
        const CDCTrajectory2D& trajectory2D,
        const CDCTrajectorySZ& trajectorySZ
      ) :
        m_startSegment(startSegment),
        m_middleSegment(middleSegment),
        m_endSegment(endSegment),
        m_trajectory2D(trajectory2D),
        m_trajectorySZ(trajectorySZ)
      {;}


      /** Destructor. */
      ~CDCSegmentTriple() {;}

      /** Equality comparision based on the pointers to the stored segments*/
      bool operator==(CDCSegmentTriple const& rhs) const {
        return getStart()  == rhs.getStart()      and
               getMiddle() == rhs.getMiddle()     and
               getEnd()    == rhs.getEnd()        ;
      }

      bool operator<(CDCSegmentTriple const& rhs) const {
        return getStart() < rhs.getStart()  or (
                 getStart() == rhs.getStart() and (
                   getEnd() < rhs.getEnd()  or (
                     getEnd() == rhs.getEnd() and (
                       getMiddle()   < rhs.getMiddle()))));
      }

      /** Equality comparision usable with points */
      bool IsEqual(const CDCSegmentTriple* const& other) const
      { return other == nullptr ? false : operator==(*other); }

      /** Establish a total ordering usable with points */
      bool IsLessThan(const CDCSegmentTriple* const& other) const
      { return other == nullptr ? false : operator<(*other); }

      static inline CDCSegmentTriple getLowerBound(const CDCAxialRecoSegment2D* startSegment)
      { return CDCSegmentTriple(startSegment, nullptr); }

      const CDCSegmentTriple* operator->() const { return this; }

      ILayerType getStartISuperLayer() const
      { return getStart() == nullptr ? INVALIDSUPERLAYER : getStart()->getISuperLayer(); }
      ILayerType getMiddleISuperLayer() const
      { return getMiddle() == nullptr ? INVALIDSUPERLAYER : getMiddle()->getISuperLayer(); }
      ILayerType getEndISuperLayer() const
      { return getEnd() == nullptr ? INVALIDSUPERLAYER : getEnd()->getISuperLayer(); }

      const CDCAxialRecoSegment2D*   getStart()   const { return m_startSegment; }
      void setStart(const CDCAxialRecoSegment2D* startSegment) { m_startSegment = startSegment; }

      const CDCStereoRecoSegment2D* getMiddle()  const { return m_middleSegment; }
      void setMiddle(const CDCStereoRecoSegment2D* middleSegment) { m_middleSegment = middleSegment; }

      const CDCAxialRecoSegment2D*   getEnd()     const { return m_endSegment; }
      void setEnd(const CDCAxialRecoSegment2D* endSegment) { m_endSegment = endSegment; }

      const  CDCTrajectory2D& getTrajectory2D() const
      { return m_trajectory2D; }

      void setTrajectory2D(const CDCTrajectory2D& trajectory2D) const
      { m_trajectory2D =  trajectory2D; }

      const CDCTrajectorySZ& getTrajectorySZ() const
      { return m_trajectorySZ; }

      void setTrajectorySZ(const CDCTrajectorySZ& trajectorySZ) const
      { m_trajectorySZ = trajectorySZ; }


      /// Sets the do not use flag of the segment triple's automaton cell and of the three contained segments
      void setDoNotUse() const {
        getAutomatonCell().setFlags(DO_NOT_USE);
        forwardDoNotUse();
      }

      /// Sets the do not use flag of the three contained segments
      void forwardDoNotUse() const {
        getStart()->getAutomatonCell().setFlags(DO_NOT_USE);
        getMiddle()->getAutomatonCell().setFlags(DO_NOT_USE);
        getEnd()->getAutomatonCell().setFlags(DO_NOT_USE);
      }

      /// If one of the contained segments is marked as do not use this segment triple is set be not usable as well
      void receiveDoNotUse() const {

        if (getStart()->getAutomatonCell().hasAnyFlags(DO_NOT_USE) or
            getMiddle()->getAutomatonCell().hasAnyFlags(DO_NOT_USE) or
            getEnd()->getAutomatonCell().hasAnyFlags(DO_NOT_USE)) {

          getAutomatonCell().setFlags(DO_NOT_USE);
        }
      }

      /// Getter for the automaton cell.
      AutomatonCell& getAutomatonCell() const { return m_automatonCell; }

    private:

      const CDCAxialRecoSegment2D* m_startSegment;
      const CDCStereoRecoSegment2D* m_middleSegment;
      const CDCAxialRecoSegment2D* m_endSegment;

      mutable CDCTrajectory2D m_trajectory2D;
      mutable CDCTrajectorySZ m_trajectorySZ;
      mutable AutomatonCell m_automatonCell;

      /** ROOT Macro to make CDCSegmentTriple a ROOT class.*/
      ClassDefInCDCLocalTracking(CDCSegmentTriple, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCSEGMENTTRIPLE
