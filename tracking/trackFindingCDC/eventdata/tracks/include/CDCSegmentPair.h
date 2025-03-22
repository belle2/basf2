/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/eventdata/trajectories/CDCTrajectory3D.h>

#include <tracking/trackingUtilities/topology/ISuperLayer.h>
#include <tracking/trackingUtilities/topology/EStereoKind.h>

#include <tracking/trackingUtilities/ca/AutomatonCell.h>

#include <tracking/trackingUtilities/numerics/EForwardBackward.h>

#include <cassert>
#include <cstddef>

namespace Belle2 {
  namespace TrackingUtilities {
    class CDCTrajectory2D;
    class CDCTrajectorySZ;
  }
  namespace TrackFindingCDC {
    class CDCSegment2D;

    /// Class representing a pair of one reconstructed axial segment and one stereo segment in
    /// adjacent superlayer.
    class CDCSegmentPair {
    public:
      /// Default constructor - for ROOT compatibility.
      CDCSegmentPair();

      /// Constructor from two segments
      CDCSegmentPair(const CDCSegment2D* fromSegment, const CDCSegment2D* toSegment);

      /// Constructor from two segments and an associated trajectory
      CDCSegmentPair(const CDCSegment2D* fromSegment,
                     const CDCSegment2D* toSegment,
                     const TrackingUtilities::CDCTrajectory3D& trajectory3D);

      /// Equality comparison based on the pointers to the stored segments.
      bool operator==(CDCSegmentPair const& rhs) const
      {
        return getFromSegment() == rhs.getFromSegment() and getToSegment() == rhs.getToSegment();
      }

      /// Total ordering scheme comparing the segment pointers.
      bool operator<(CDCSegmentPair const& rhs) const
      {
        return (getFromSegment() < rhs.getFromSegment() or
                (getFromSegment() == rhs.getFromSegment() and getToSegment() < rhs.getToSegment()));
      }

      /// Define reconstructed segments and axial stereo segment pairs as coaligned on the from
      /// segment
      friend bool operator<(const CDCSegmentPair& segmentPair, const CDCSegment2D* segment)
      {
        return segmentPair.getFromSegment() < segment;
      }

      /// Define reconstructed segments and axial stereo segment pairs as coaligned on the from
      /// segment
      friend bool operator<(const CDCSegment2D* segment, const CDCSegmentPair& segmentPair)
      {
        return segment < segmentPair.getFromSegment();
      }

      /// Checks if both stored segments are not nullptr. Returns true if check is succeeded.
      bool checkSegmentsNonNullptr() const
      {
        return getFromSegment() != nullptr and getToSegment() != nullptr;
      }

      /// Checks if the two segments are of different axial type.
      bool checkSegmentsStereoKinds() const
      {
        return (getFromStereoKind() == TrackingUtilities::EStereoKind::c_Axial) xor
               (getToStereoKind() == TrackingUtilities::EStereoKind::c_Axial);
      }

      /// Checks the references to the contained three segment for nullptrs and exactly one of them
      /// is axial and one is stereo
      bool checkSegments() const
      {
        return checkSegmentsNonNullptr() and checkSegmentsStereoKinds();
      }

      /// Getter for the stereo type of the first segment.
      TrackingUtilities::EStereoKind getFromStereoKind() const;

      /// Getter for the stereo type of the second segment.
      TrackingUtilities::EStereoKind getToStereoKind() const;

      /// Getter for the superlayer id of the from segment.
      TrackingUtilities::ISuperLayer getFromISuperLayer() const;

      /// Getter for the superlayer id of the to segment.
      TrackingUtilities::ISuperLayer getToISuperLayer() const;

      /// Getter for the total number of hits in this segment pair
      std::size_t size() const;

      /// Getter for the from segment.
      const CDCSegment2D* getFromSegment() const
      {
        return m_fromSegment;
      }

      /// Setter for the from segment.
      void setFromSegment(const CDCSegment2D* fromSegment)
      {
        setSegments(fromSegment, getToSegment());
      }

      /// Getter for the to segment.
      const CDCSegment2D* getToSegment() const
      {
        return m_toSegment;
      }

      /// Setter for the to segment.
      void setToSegment(const CDCSegment2D* toSegment)
      {
        setSegments(getFromSegment(), toSegment);
      }

      /// Getter for the stereo segment
      const CDCSegment2D* getStereoSegment() const
      {
        return getFromStereoKind() != TrackingUtilities::EStereoKind::c_Axial ? getFromSegment() : getToSegment();
      }

      /// Getter for the axial segment
      const CDCSegment2D* getAxialSegment() const
      {
        return getFromStereoKind() == TrackingUtilities::EStereoKind::c_Axial ? getFromSegment() : getToSegment();
      }

      /// Setter for both segments simultaneously
      void setSegments(const CDCSegment2D* fromSegment, const CDCSegment2D* toSegment)
      {
        m_fromSegment = fromSegment;
        m_toSegment = toSegment;
        assert(checkSegmentsNonNullptr());
        assert(checkSegmentsStereoKinds());
      }

      /** Determines the angle between the last reconstructed position of the from segment and
       *  the first reconstructed position of the to segment.*/
      double computeDeltaPhiAtSuperLayerBound() const;

      /** Indicator if the from segment lies before the to segment */
      double computeFromIsBeforeTo() const
      {
        return computeFromIsBeforeToFitless();
      }

      /** Indicator if the from segment lies before the to segment */
      double computeToIsAfterFrom() const
      {
        return computeToIsAfterFromFitless();
      }

      /** Indicator if the from segment and the to segment have roughly the same travel direction.*/
      double computeIsCoaligned() const
      {
        return computeIsCoalignedFitless();
      }

      /** Indicator if the from segment lies before the to segment, build without using the
       * trajectories, which may not have been fitted yet. */
      double computeFromIsBeforeToFitless() const;

      /** Indicator if the from segment lies before the to segment, build without using the
       * trajectories, which may not have been fitted yet. */
      double computeToIsAfterFromFitless() const;

      /** Indicator if the from segment and the to segment have roughly the same travel direction
       * without using the common fit*/
      double computeIsCoalignedFitless() const;

      /// Checks if the last entity in the vector lies greater or lower travel distance than the
      /// last entity.
      /** Returns:
       *  * EForwardBackward::c_Forward if the last entity lies behind the first.
       *  * EForwardBackward::c_Backward if the last entity lies before the first.
       */
      TrackingUtilities::EForwardBackward isCoaligned(const TrackingUtilities::CDCTrajectory2D& trajectory2D) const;

      /// Setter for the three dimensional trajectory
      void setTrajectory3D(const TrackingUtilities::CDCTrajectory3D& trajectory3D) const
      {
        m_trajectory3D = trajectory3D;
      }

      /// Getter for the three dimensional trajectory.
      TrackingUtilities::CDCTrajectory3D& getTrajectory3D() const
      {
        return m_trajectory3D;
      }

      /// Getter for the two dimensional projection of the common three dimensional trajectory.
      TrackingUtilities::CDCTrajectory2D getTrajectory2D() const;

      /// Getter for the sz projection of the common three dimensional trajectory.
      TrackingUtilities::CDCTrajectorySZ getTrajectorySZ() const;

      /// Invalides the currently stored trajectory information.
      void clearTrajectory3D() const
      {
        getTrajectory3D().clear();
      }

      /// Unsets the masked flag of the segment triple's automaton cell and of the three contained
      /// segments.
      void unsetAndForwardMaskedFlag() const;

      /// Sets the masked flag of the segment triple's automaton cell and of the three contained
      /// segments.
      void setAndForwardMaskedFlag() const;

      /// If one of the contained segments is marked as masked this segment triple is set be masked
      /// as well.
      void receiveMaskedFlag() const;

      /// Mutable getter for the automaton cell.
      TrackingUtilities::AutomatonCell& getAutomatonCell() const
      {
        return m_automatonCell;
      }

    private:
      /// Reference to the from segment
      const CDCSegment2D* m_fromSegment;

      /// Reference to the to segment
      const CDCSegment2D* m_toSegment;

      /// Memory for the common three dimensional trajectory
      mutable TrackingUtilities::CDCTrajectory3D m_trajectory3D;

      /// Automaton cell associated with the pair of segments
      mutable TrackingUtilities::AutomatonCell m_automatonCell;
    };
  }
}
