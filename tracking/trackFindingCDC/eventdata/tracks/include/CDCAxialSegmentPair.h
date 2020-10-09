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

#include <tracking/trackFindingCDC/eventdata/segments/CDCAxialSegment2D.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

#include <tracking/trackFindingCDC/topology/ISuperLayer.h>

#include <tracking/trackFindingCDC/ca/AutomatonCell.h>

#include <tracking/trackFindingCDC/numerics/EForwardBackward.h>

#include <tuple>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegment2D;

    /// Class representing a pair of reconstructed axial segements in adjacent superlayer
    class CDCAxialSegmentPair  {

    public:
      /// Default constructor setting the contained segmetns to nullptr.
      CDCAxialSegmentPair();

      /// Constructor from two segments
      CDCAxialSegmentPair(const CDCAxialSegment2D* startSegment,
                          const CDCAxialSegment2D* endSegment);

      /// Constructor from two segments and an assoziated trajectory
      CDCAxialSegmentPair(const CDCAxialSegment2D* startSegment,
                          const CDCAxialSegment2D* endSegment,
                          const CDCTrajectory2D& trajectory2D);

      /// Equality comparision based on the pointers to the stored segments.
      bool operator==(CDCAxialSegmentPair const& rhs) const
      {
        return
          std::tie(m_startSegment, m_endSegment) ==
          std::tie(rhs.m_startSegment, rhs.m_endSegment);
      }

      /// Total ordering sheme comparing the segment pointers.
      bool operator<(CDCAxialSegmentPair const& rhs) const
      {
        return
          std::tie(m_startSegment, m_endSegment) <
          std::tie(rhs.m_startSegment, rhs.m_endSegment);
      }

      /// Define reconstructed segments and segment triples as coaligned on the start segment
      friend bool
      operator<(CDCAxialSegmentPair const& segmentPair, const CDCAxialSegment2D* axialSegment)
      {
        return segmentPair.getStartSegment() < axialSegment;
      }

      /// Define reconstructed segments and segment pairs as coaligned on the start segment
      friend bool
      operator<(const CDCAxialSegment2D* axialSegment, CDCAxialSegmentPair const& segmentPair)
      {
        return axialSegment < segmentPair.getStartSegment();
      }

      /// Checks the references to the contained three segment for nullptrs.
      bool checkSegments() const
      {
        return not(m_startSegment == nullptr or m_endSegment == nullptr);
      }

      /// Getter for the superlayer id of the start segment
      ISuperLayer getStartISuperLayer() const;

      /// Getter for the superlayer id of the end segment
      ISuperLayer getEndISuperLayer() const;

      /// Getter for the start segment.
      const CDCAxialSegment2D* getStartSegment() const
      {
        return m_startSegment;
      }

      /// Setter for the start segment.
      void setStartSegment(const CDCAxialSegment2D* startSegment)
      {
        m_startSegment = startSegment;
      }

      /// Getter for the end segment.
      const CDCAxialSegment2D* getEndSegment() const
      {
        return m_endSegment;
      }

      /// Setter for the end segment.
      void setEndSegment(const CDCAxialSegment2D* endSegment)
      {
        m_endSegment = endSegment;
      }

      /// Setter for both segments simultaniously
      void setSegments(const CDCSegment2D* startSegment, const CDCSegment2D* endSegment)
      {
        m_startSegment = startSegment;
        m_endSegment = endSegment;
        checkSegments();
      }

      /// Getter for the trajectory of the two dimensional trajectory
      CDCTrajectory2D& getTrajectory2D() const
      {
        return m_trajectory2D;
      }

      /// Setter for the trajectory of the two dimensional trajectory
      void setTrajectory2D(const CDCTrajectory2D& trajectory2D) const
      { m_trajectory2D =  trajectory2D; }

      /// Invalides the currently stored trajectory information
      void clearTrajectory2D() const
      { getTrajectory2D().clear(); }


      /// Unsets the masked flag of the segment pair's automaton cell, of the contained segments and of the contained wire hits.
      void unsetAndForwardMaskedFlag() const;

      /// Sets the masked flag of the segment pair's automaton cell. Also forward the masked flag to the contained segments and the contained wire hits.
      void setAndForwardMaskedFlag() const;

      /// If one of the contained segments is marked as masked this segment pair is set be masked as well.
      void receiveMaskedFlag() const;

      /// Checks if the last entity in the vector lies greater or lower travel distance than the last entity.
      /** Returns:
       *  * EForwardBackward::c_Forward if the last entity lies behind the first.
       *  * EForwardBackward::c_Backward if the last entity lies before the first.
       */
      EForwardBackward isCoaligned(const CDCTrajectory2D& trajectory2D) const;

      /// Mutable getter for the automaton cell.
      AutomatonCell& getAutomatonCell() const
      {
        return m_automatonCell;
      }

    private:
      /// Reference to the start segment
      const CDCAxialSegment2D* m_startSegment;

      /// Reference to the end segment
      const CDCAxialSegment2D* m_endSegment;

      /// Reference to the common trajectory
      mutable CDCTrajectory2D m_trajectory2D;

      /// Automaton cell assoziated with the pair of segments
      mutable AutomatonCell m_automatonCell;
    };
  }
}
