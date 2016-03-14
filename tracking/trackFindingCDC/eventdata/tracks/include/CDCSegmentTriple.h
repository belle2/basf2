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
#include <tracking/trackFindingCDC/eventdata/segments/CDCAxialRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCStereoRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>

#include <tracking/trackFindingCDC/ca/AutomatonCell.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class representing a triple of reconstructed segements in adjacent superlayer
    class CDCSegmentTriple {
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
                       const CDCTrajectory3D& trajector3D);

      /// Equality comparision based on the pointers to the stored segments
      bool operator==(CDCSegmentTriple const& rhs) const
      {
        return
          std::tie(m_startSegment, m_middleSegment, m_endSegment) ==
          std::tie(rhs.m_startSegment, rhs.m_middleSegment, rhs.m_endSegment);
      }

      /// Total ordering sheme based on the two axial segments first and the stereo segments second
      bool operator<(CDCSegmentTriple const& rhs) const
      {
        return
          std::tie(m_startSegment, m_middleSegment, m_endSegment) <
          std::tie(rhs.m_startSegment, rhs.m_middleSegment, rhs.m_endSegment);
      }

      /// Define reconstructed segments and segment triples as coaligned on the start segment
      friend bool operator<(CDCSegmentTriple const& segmentTriple,
                            const CDCAxialRecoSegment2D* axialSegment)
      { return segmentTriple.getStartSegment() < axialSegment; }

      /// Define reconstructed segments and segment triples as coaligned on the start segment
      friend bool operator<(const CDCAxialRecoSegment2D* axialSegment,
                            CDCSegmentTriple const& segmentTriple)
      { return axialSegment < segmentTriple.getStartSegment(); }

      /// Checks the references to the contained three segment for nullptrs
      bool checkSegments() const
      { return not(getStartSegment() == nullptr) and not(m_middleSegment == nullptr) and not(m_endSegment == nullptr); }

      /// Getter for the superlayer id of the start segment
      ISuperLayer getStartISuperLayer() const
      { return ISuperLayerUtil::getFrom(getStartSegment()); }

      /// Getter for the superlayer id of the middle segment
      ISuperLayer getMiddleISuperLayer() const
      { return ISuperLayerUtil::getFrom(getMiddleSegment()); }

      /// Getter for the superlayer id of the end segment
      ISuperLayer getEndISuperLayer() const
      { return ISuperLayerUtil::getFrom(getEndSegment()); }


      /// Getter for the start axial segment.
      const CDCAxialRecoSegment2D* getStartSegment() const
      { return m_startSegment; }

      /// Setter for the start axial segment.
      void setStartSegment(const CDCAxialRecoSegment2D* startSegment)
      { m_startSegment = startSegment; }

      /// Getter for the middle stereo segment
      const CDCStereoRecoSegment2D* getMiddleSegment()  const
      { return m_middleSegment; }

      /// Setter for the middle stereo segment
      void setMiddleSegment(const CDCStereoRecoSegment2D* middleSegment)
      { m_middleSegment = middleSegment; }

      /// Getter for the end axial segment.
      const CDCAxialRecoSegment2D* getEndSegment() const
      { return m_endSegment; }

      /// Setter for the end axial segment.
      void setEndSegment(const CDCAxialRecoSegment2D* endSegment)
      { m_endSegment = endSegment; }


      /// Getter for the linear trajectory in the sz direction.
      CDCTrajectorySZ getTrajectorySZ() const
      { return getTrajectory3D().getTrajectorySZ(); }

      /// Getter for the circular trajectory in the xy direction.
      CDCTrajectory2D getTrajectory2D() const
      { return getTrajectory3D().getTrajectory2D(); }

      /// Getter for the three dimensional helix trajectory
      const CDCTrajectory3D& getTrajectory3D() const
      { return m_trajectory3D; }

      /// Setter for the three dimensional helix trajectory
      void setTrajectory3D(const CDCTrajectory3D& trajectory3D) const
      { m_trajectory3D = trajectory3D; }

      /// Clears the three dimensional helix trajectory
      void clearTrajectory3D() const
      { m_trajectory3D.clear(); }

      /// Unsets the masked flag of the segment triple's automaton cell, of the contained segments and of the contained wire hits.
      void unsetAndForwardMaskedFlag() const
      {
        getAutomatonCell().unsetMaskedFlag();
        getStartSegment()->unsetAndForwardMaskedFlag();
        getMiddleSegment()->unsetAndForwardMaskedFlag();
        getEndSegment()->unsetAndForwardMaskedFlag();
      }

      /// Sets the masked flag of the segment triple's automaton cell. Also forward the masked to the contained segments and the contained wire hits.
      void setAndForwardMaskedFlag() const
      {
        getAutomatonCell().setMaskedFlag();
        getStartSegment()->setAndForwardMaskedFlag();
        getMiddleSegment()->setAndForwardMaskedFlag();
        getEndSegment()->setAndForwardMaskedFlag();
      }

      /// If one of the contained segments is marked as masked this segment triple is set be masked as well.
      void receiveMaskedFlag() const
      {
        getStartSegment()->receiveMaskedFlag();
        getMiddleSegment()->receiveMaskedFlag();
        getEndSegment()->receiveMaskedFlag();

        if (getStartSegment()->getAutomatonCell().hasMaskedFlag() or
            getMiddleSegment()->getAutomatonCell().hasMaskedFlag() or
            getEndSegment()->getAutomatonCell().hasMaskedFlag()) {

          getAutomatonCell().setMaskedFlag();

        }
      }

      /// Getter for the automaton cell.
      AutomatonCell& getAutomatonCell() { return m_automatonCell; }

      /// Constant getter for the automaton cell.
      const AutomatonCell& getAutomatonCell() const { return m_automatonCell; }

    private:
      /// Reference to the axial segment in the start of the triple.
      const CDCRecoSegment2D* m_startSegment;

      /// Reference to the stereo segment in the middle of the triple.
      const CDCStereoRecoSegment2D* m_middleSegment;

      /// Reference to the axial segment in the start of the triple.
      const CDCAxialRecoSegment2D* m_endSegment;

      /// Memory of the linear trajectory in the sz direction assoziated with the triple.
      mutable CDCTrajectory3D m_trajectory3D;

      /// Automaton cell assoziated with the pair of segments
      mutable AutomatonCell m_automatonCell;

    }; // class CDCSegmentTriple
  } // namespace TrackFindingCDC
} // namespace Belle2
