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

#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCStereoRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>

#include <tracking/trackFindingCDC/ca/AutomatonCell.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class representing a triple of reconstructed segements in adjacent superlayer
    class CDCSegmentTriple : public CDCAxialSegmentPair {
    public:

      /// Default constructor for ROOT compatability
      CDCSegmentTriple();

      /// Constructor taking two axial segments leaving the middle stereo segment set to null pointer.
      CDCSegmentTriple(const CDCAxialRecoSegment2D* startSegment,
                       const CDCAxialRecoSegment2D* endSegment);

      /// Constructor taking two axial segments leaving the middle stereo segment set to null pointer.
      CDCSegmentTriple(const CDCAxialSegmentPair& segmentPair);

      /// Constructor taking the three segments the triple shall be made of.
      CDCSegmentTriple(const CDCAxialRecoSegment2D* startSegment,
                       const CDCStereoRecoSegment2D* middleSegment,
                       const CDCAxialRecoSegment2D* endSegment);

      /// Constructor taking the three segments the triple shall be made of and the two dimensional and sz trajectory.
      CDCSegmentTriple(const CDCAxialRecoSegment2D* startSegment,
                       const CDCStereoRecoSegment2D* middleSegment,
                       const CDCAxialRecoSegment2D* endSegment,
                       const CDCTrajectory2D& trajectory2D,
                       const CDCTrajectorySZ& trajectorySZ);

      /// Equality comparision based on the pointers to the stored segments
      bool operator==(CDCSegmentTriple const& rhs) const
      { return CDCAxialSegmentPair::operator==(rhs) and getMiddle() == rhs.getMiddle(); }

      /// Total ordering sheme based on the two axial segments first and the stereo segments second
      bool operator<(CDCSegmentTriple const& rhs) const
      {
        return (CDCAxialSegmentPair::operator<(rhs) or
                (CDCAxialSegmentPair::operator==(rhs) and getMiddle() < rhs.getMiddle()));
      }

      /// Define reconstructed segments and segment triples as coaligned on the start segment
      friend bool operator<(CDCSegmentTriple const& segmentTriple,
                            const CDCAxialRecoSegment2D* axialSegment)
      { return segmentTriple.getStart() < axialSegment; }

      /// Define reconstructed segments and segment triples as coaligned on the start segment
      friend bool operator<(const CDCAxialRecoSegment2D* axialSegment,
                            CDCSegmentTriple const& segmentTriple)
      { return axialSegment < segmentTriple.getStart(); }

      /// Checks the references to the contained three segment for nullptrs
      bool checkSegments() const
      { return CDCAxialSegmentPair::checkSegments() and not(m_middleSegment == nullptr); }

      /// Getter for the superlayer id of the middle segment
      ISuperLayer getMiddleISuperLayer() const
      { return getMiddle() == nullptr ? ISuperLayerUtil::c_Invalid : getMiddle()->getISuperLayer(); }

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
      { clearTrajectorySZ(); CDCAxialSegmentPair::clearTrajectory2D(); }

      /// Unsets the masked flag of the segment triple's automaton cell, of the contained segments and of the contained wire hits.
      void unsetAndForwardMaskedFlag() const
      {
        getAutomatonCell().unsetMaskedFlag();
        CDCAxialSegmentPair::unsetAndForwardMaskedFlag();
        getMiddle()->unsetAndForwardMaskedFlag();
      }

      /// Sets the masked flag of the segment triple's automaton cell. Also forward the masked to the contained segments and the contained wire hits.
      void setAndForwardMaskedFlag() const
      {
        getAutomatonCell().setMaskedFlag();
        CDCAxialSegmentPair::setAndForwardMaskedFlag();
        getMiddle()->setAndForwardMaskedFlag();
      }

      /// If one of the contained segments is marked as masked this segment triple is set be masked as well.
      void receiveMaskedFlag() const
      {
        CDCAxialSegmentPair::receiveMaskedFlag();
        getMiddle()->receiveMaskedFlag();

        if (getStart()->getAutomatonCell().hasMaskedFlag() or
            getMiddle()->getAutomatonCell().hasMaskedFlag() or
            getEnd()->getAutomatonCell().hasMaskedFlag()) {

          getAutomatonCell().setMaskedFlag();

        }
      }

    private:
      /// Reference to the stereo segment in the middle of the triple.
      const CDCStereoRecoSegment2D* m_middleSegment;

      /// Memory of the linear trajectory in the sz direction assoziated with the triple.
      mutable CDCTrajectorySZ m_trajectorySZ;


    }; //class

  } // namespace TrackFindingCDC
} // namespace Belle2
