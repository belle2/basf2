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

#include <tracking/cdcLocalTracking/eventdata/tracks/CDCAxialAxialSegmentPair.h>



namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class representing a triple of reconstructed segements in adjacent superlayer
    class CDCSegmentTriple : public CDCAxialAxialSegmentPair {
    public:

      /// Default constructor for ROOT compatability
      CDCSegmentTriple();

      /// Constructor taking two axial segments leaving the middle stereo segment set to null pointer.
      CDCSegmentTriple(const CDCAxialRecoSegment2D* startSegment, const CDCAxialRecoSegment2D* endSegment);

      /// Constructor taking the three segments the triple shall be made of.
      CDCSegmentTriple(
        const CDCAxialRecoSegment2D* startSegment,
        const CDCStereoRecoSegment2D* middleSegment,
        const CDCAxialRecoSegment2D* endSegment
      );

      /// Constructor taking the three segments the triple shall be made of and the two dimensional and sz trajectory.
      CDCSegmentTriple(
        const CDCAxialRecoSegment2D* startSegment,
        const CDCStereoRecoSegment2D* middleSegment,
        const CDCAxialRecoSegment2D* endSegment,
        const CDCTrajectory2D& trajectory2D,
        const CDCTrajectorySZ& trajectorySZ
      );

      /// Empty destructor
      ~CDCSegmentTriple();

      /// Equality comparision based on the pointers to the stored segments
      bool operator==(CDCSegmentTriple const& rhs) const {
        return CDCAxialAxialSegmentPair::operator==(rhs) and getMiddle() == rhs.getMiddle();
      }

      /// Total ordering sheme based on the two axial segments first and the stereo segments second
      bool operator<(CDCSegmentTriple const& rhs) const {
        return  CDCAxialAxialSegmentPair::operator<(rhs) or (CDCAxialAxialSegmentPair::operator==(rhs) and getMiddle() < rhs.getMiddle());
      }

      /// Define reconstructed segments and segment triples as coaligned on the start segment
      friend bool operator<(CDCSegmentTriple const& segmentTriple , const CDCAxialRecoSegment2D* axialSegment)
      { return segmentTriple.getStart() < axialSegment; }

      /// Define reconstructed segments and segment triples as coaligned on the start segment
      friend bool operator<(const CDCAxialRecoSegment2D* axialSegment, CDCSegmentTriple const& segmentTriple)
      { return axialSegment < segmentTriple.getStart(); }

      /// Checks the references to the contained three segment for nullptrs
      bool checkSegments() const
      { return CDCAxialAxialSegmentPair::checkSegments() and not(m_middleSegment == nullptr); }

      /// Getter for the superlayer id of the middle segment
      ILayerType getMiddleISuperLayer() const
      { return getMiddle() == nullptr ? INVALIDSUPERLAYER : getMiddle()->getISuperLayer(); }

      /// Getter for the middle stereo segment
      const CDCStereoRecoSegment2D* getMiddle()  const
      { return m_middleSegment; }

      /// Setter for the middle stereo segment
      void setMiddle(const CDCStereoRecoSegment2D* middleSegment)
      { m_middleSegment = middleSegment; }

      /// Getter for the linear trajectory in the sz direction.
      CDCTrajectorySZ& getTrajectorySZ() const
      { return m_trajectorySZ; }

      /// Setter for the linear trajectory in the sz direction.
      void setTrajectorySZ(const CDCTrajectorySZ& trajectorySZ) const
      { m_trajectorySZ = trajectorySZ; }

      /// Clears the linear trajectory in the sz direction.
      void clearTrajectorySZ() const
      { getTrajectorySZ().clear(); }

      /// Clears all stored trajectories to an invalid state
      void clearTrajectories() const
      { clearTrajectorySZ(); CDCAxialAxialSegmentPair::clearTrajectory2D(); }

      /// Sets the do not use flag of the segment triple's automaton cell and of the three contained segments
      void setDoNotUse() const {
        getAutomatonCell().setDoNotUseFlag();
        forwardDoNotUse();
      }

      /// Sets the do not use flag of the three contained segments
      void forwardDoNotUse() const {
        CDCAxialAxialSegmentPair::forwardDoNotUse();
        getMiddle()->getAutomatonCell().setDoNotUseFlag();
      }

      /// If one of the contained segments is marked as do not use this segment triple is set be not usable as well - returns an indicator, if this cell is not usable anymore.
      void receiveDoNotUse() const {

        if (getStart()->getAutomatonCell().hasDoNotUseFlag() or
            getMiddle()->getAutomatonCell().hasDoNotUseFlag() or
            getEnd()->getAutomatonCell().hasDoNotUseFlag()) {

          getAutomatonCell().setDoNotUseFlag();

        }
      }

    private:
      const CDCStereoRecoSegment2D* m_middleSegment;  ///< Reference to the stereo segment in the middle of the triple.
      mutable CDCTrajectorySZ m_trajectorySZ; ///< Memory of the linear trajectory in the sz direction assoziated with the triple.

      /** ROOT Macro to make CDCSegmentTriple a ROOT class.*/
      ClassDefInCDCLocalTracking(CDCSegmentTriple, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCSEGMENTTRIPLE
