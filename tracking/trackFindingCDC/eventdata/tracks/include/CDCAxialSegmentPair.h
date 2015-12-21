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

#include <tracking/trackFindingCDC/eventdata/segments/CDCAxialRecoSegment2D.h>

#include <tracking/trackFindingCDC/ca/AutomatonCell.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class representing a pair of reconstructed axial segements in adjacent superlayer
    class CDCAxialSegmentPair  {
    public:

      /// Default constructor - for ROOT compatability
      CDCAxialSegmentPair();

      /// Constructor from two segments
      CDCAxialSegmentPair(const CDCAxialRecoSegment2D* startSegment,
                          const CDCAxialRecoSegment2D* endSegment);

      /// Constructor from two segments and an assoziated trajectory
      CDCAxialSegmentPair(const CDCAxialRecoSegment2D* startSegment,
                          const CDCAxialRecoSegment2D* endSegment,
                          const CDCTrajectory2D& trajectory2D);

      /// Equality comparision based on the pointers to the stored segments.
      bool operator==(CDCAxialSegmentPair const& rhs) const
      { return getStart() == rhs.getStart() and getEnd() == rhs.getEnd(); }

      /// Total ordering sheme comparing the segment pointers.
      bool operator<(CDCAxialSegmentPair const& rhs) const
      {
        return (getStart() < rhs.getStart()  or
                (getStart() == rhs.getStart() and getEnd() < rhs.getEnd()));
      }

      /// Define reconstructed segments and segment triples as coaligned on the start segment
      friend bool operator<(CDCAxialSegmentPair const& segmentTriple,
                            const CDCAxialRecoSegment2D* axialSegment)
      { return segmentTriple.getStart() < axialSegment; }

      /// Define reconstructed segments and segment triples as coaligned on the start segment
      friend bool operator<(const CDCAxialRecoSegment2D* axialSegment,
                            CDCAxialSegmentPair const& segmentTriple)
      { return axialSegment < segmentTriple.getStart(); }


      /// Checks the references to the contained three segment for nullptrs.
      bool checkSegments() const
      { return not(m_startSegment == nullptr or m_endSegment == nullptr); }


      /// Getter for the superlayer id of the start segment.
      ISuperLayer getStartISuperLayer() const
      { return getStart() == nullptr ? ISuperLayerUtil::c_Invalid : getStart()->getISuperLayer(); }

      /// Getter for the superlayer id of the end segment.
      ISuperLayer getEndISuperLayer() const
      { return getEnd() == nullptr ? ISuperLayerUtil::c_Invalid : getEnd()->getISuperLayer(); }

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


      /// Unsets the masked flag of the segment pair's automaton cell, of the contained segments and of the contained wire hits.
      void unsetAndForwardMaskedFlag() const
      {
        getAutomatonCell().unsetMaskedFlag();
        getStart()->unsetAndForwardMaskedFlag();
        getEnd()->unsetAndForwardMaskedFlag();
      }

      /// Sets the masked flag of the segment pair's automaton cell. Also forward the masked flag to the contained segments and the contained wire hits.
      void setAndForwardMaskedFlag() const
      {
        getAutomatonCell().setMaskedFlag();
        getStart()->setAndForwardMaskedFlag();
        getEnd()->setAndForwardMaskedFlag();
      }

      /// If one of the contained segments is marked as masked this segment triple is set be masked as well.
      void receiveMaskedFlag() const
      {
        getStart()->receiveMaskedFlag();
        getEnd()->receiveMaskedFlag();

        if (getStart()->getAutomatonCell().hasMaskedFlag() or
            getEnd()->getAutomatonCell().hasMaskedFlag()) {
          getAutomatonCell().setMaskedFlag();
        }

      }

      /// Checks if the last entity in the vector lies greater or lower travel distance than the last entity.
      /** Returns:
       *  * EForwardBackward::c_Forward if the last entity lies behind the first.
       *  * EForwardBackward::c_Backward if the last entity lies before the first.
       */
      EForwardBackward isCoaligned(const CDCTrajectory2D& trajectory2D) const
      {
        EForwardBackward startIsCoaligned = trajectory2D.isForwardOrBackwardTo(*(getStart()));
        EForwardBackward endIsCoaligned = trajectory2D.isForwardOrBackwardTo(*(getEnd()));
        if (startIsCoaligned == EForwardBackward::c_Forward and
            endIsCoaligned == EForwardBackward::c_Forward) {
          return EForwardBackward::c_Forward;
        } else if (startIsCoaligned == EForwardBackward::c_Backward and
                   endIsCoaligned == EForwardBackward::c_Backward) {
          return EForwardBackward::c_Backward;
        } else {
          return EForwardBackward::c_Invalid;
        }
      }

      /// Getter for the automaton cell.
      AutomatonCell& getAutomatonCell() { return m_automatonCell; }

      /// Constant getter for the automaton cell.
      const AutomatonCell& getAutomatonCell() const { return m_automatonCell; }

    private:
      /// Reference to the start segment
      const CDCAxialRecoSegment2D* m_startSegment;

      /// Reference to the end segment
      const CDCAxialRecoSegment2D* m_endSegment;

      /// Reference to the common trajectory
      mutable CDCTrajectory2D m_trajectory2D;

      /// Automaton cell assoziated with the pair of segments
      mutable AutomatonCell m_automatonCell;

    }; //end class CDCAxialSegmentPair

  } // end namespace TrackFindingCDC
} // end namespace Belle2
