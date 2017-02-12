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

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>

#include <tracking/trackFindingCDC/ca/AutomatonCell.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class representing a pair of one reconstructed axial segement and one stereo segment in adjacent superlayer.
    class CDCSegmentPair  {
    public:

      /// Default constructor - for ROOT compatability.
      CDCSegmentPair();

      /// Constructor from two segments
      CDCSegmentPair(const CDCSegment2D* fromSegment,
                     const CDCSegment2D* toSegment);

      /// Constructor from two segments and an assoziated trajectory
      CDCSegmentPair(const CDCSegment2D* fromSegment,
                     const CDCSegment2D* toSegment,
                     const CDCTrajectory3D& trajectory3D);

      /// Equality comparision based on the pointers to the stored segments.
      bool operator==(CDCSegmentPair const& rhs) const
      { return getFromSegment() == rhs.getFromSegment() and getToSegment() == rhs.getToSegment(); }



      /// Total ordering sheme comparing the segment pointers.
      bool operator<(CDCSegmentPair const& rhs) const
      {
        return (getFromSegment() < rhs.getFromSegment()  or
                (getFromSegment() == rhs.getFromSegment() and getToSegment() < rhs.getToSegment()));
      }

      /// Define reconstructed segments and axial stereo segment pairs as coaligned on the from segment
      friend bool operator<(const CDCSegmentPair& segmentPair,
                            const CDCSegment2D* segment)
      { return segmentPair.getFromSegment() < segment; }

      /// Define reconstructed segments and axial stereo segment pairs as coaligned on the from segment
      friend bool operator<(const CDCSegment2D* segment,
                            const CDCSegmentPair& segmentPair)
      { return segment < segmentPair.getFromSegment(); }



      /// Checks if both stored segments are not nullptr. Returns true if check is succeded.
      bool checkSegmentsNonNullptr() const
      { return getFromSegment() != nullptr and getToSegment() != nullptr; }


      /// Checks if the two segments are of different axial type.
      bool checkSegmentsStereoKinds() const
      { return (getFromStereoKind() == EStereoKind::c_Axial) xor(getToStereoKind() == EStereoKind::c_Axial); }

      /// Checks the references to the contained three segment for nullptrs and exactly one of them is axial and one is stereo
      bool checkSegments() const
      { return checkSegmentsNonNullptr() and checkSegmentsStereoKinds(); }

      /// Getter for the stereo type of the first segment.
      EStereoKind getFromStereoKind() const
      {
        return getFromSegment() == nullptr ? EStereoKind::c_Invalid
               : getFromSegment()->back().getStereoKind();
      }

      /// Getter for the stereo type of the second segment.
      EStereoKind getToStereoKind() const
      {
        return getToSegment() == nullptr ? EStereoKind::c_Invalid
               : getToSegment()->front().getStereoKind();
      }

      /// Getter for the superlayer id of the from segment.
      ISuperLayer getFromISuperLayer() const
      {
        return getFromSegment() == nullptr ? ISuperLayerUtil::c_Invalid
               : getFromSegment()->back().getISuperLayer();
      }

      /// Getter for the superlayer id of the to segment.
      ISuperLayer getToISuperLayer() const
      {
        return getToSegment() == nullptr ? ISuperLayerUtil::c_Invalid
               : getToSegment()->front().getISuperLayer();
      }

      /// Getter for the total number of hits in this segment pair
      std::size_t size() const
      { return getFromSegment()->size() + getToSegment()->size(); }

      /// Getter for the from segment.
      const CDCSegment2D* getFromSegment() const
      { return m_fromSegment; }

      /// Setter for the from segment.
      void setFromSegment(const CDCSegment2D* fromSegment)
      { setSegments(fromSegment, getToSegment()); }



      /// Getter for the to segment.
      const CDCSegment2D* getToSegment() const
      { return m_toSegment; }

      /// Setter for the to segment.
      void setToSegment(const CDCSegment2D* toSegment)
      { setSegments(getFromSegment(), toSegment); }



      /// Getter for the stereo segment
      const CDCSegment2D* getStereoSegment() const
      { return getFromStereoKind() != EStereoKind::c_Axial ? getFromSegment() : getToSegment(); }

      /// Getter for the axial segment
      const CDCSegment2D* getAxialSegment() const
      { return getFromStereoKind() == EStereoKind::c_Axial ? getFromSegment() : getToSegment(); }


      /// Setter for both segments simultaniously
      void setSegments(const CDCSegment2D* fromSegment, const CDCSegment2D* toSegment)
      {
        m_fromSegment = fromSegment;
        m_toSegment = toSegment;
        if (not checkSegmentsNonNullptr()) B2WARNING("CDCSegmentPair: one segment set to nullptr");
        if (not checkSegmentsStereoKinds()) B2WARNING("CDCSegmentPair: segments set with same stereo type");
      }

      /** Determines the angle between the last reconstructed position of the from segment and
       *  the first reconstructed position of the to segment.*/
      double computeDeltaPhiAtSuperLayerBound() const;

      /** Indicator if the from segment lies before the to segment */
      double computeFromIsBeforeTo() const
      { return computeFromIsBeforeToFitless(); }

      /** Indicator if the from segment lies before the to segment */
      double computeToIsAfterFrom() const
      { return computeToIsAfterFromFitless(); }

      /** Indicator if the from segment and the to segment have roughly the same travel direction.*/
      double computeIsCoaligned() const
      { return computeIsCoalignedFitless(); }

      /** Indicator if the from segment lies before the to segment, build without using the trajectories, which may not have been fitted yet. */
      double computeFromIsBeforeToFitless() const;

      /** Indicator if the from segment lies before the to segment, build without using the trajectories, which may not have been fitted yet. */
      double computeToIsAfterFromFitless() const;

      /** Indicator if the from segment and the to segment have roughly the same travel direction without using the common fit*/
      double computeIsCoalignedFitless() const;

      /// Checks if the last entity in the vector lies greater or lower travel distance than the last entity.
      /** Returns:
       *  * EForwardBackward::c_Forward if the last entity lies behind the first.
       *  * EForwardBackward::c_Backward if the last entity lies before the first.
       */
      EForwardBackward isCoaligned(const CDCTrajectory2D& trajectory2D) const
      {
        EForwardBackward fromIsCoaligned = trajectory2D.isForwardOrBackwardTo(*(getFromSegment()));
        EForwardBackward toIsCoaligned = trajectory2D.isForwardOrBackwardTo(*(getToSegment()));

        if (fromIsCoaligned == EForwardBackward::c_Forward and
            toIsCoaligned == EForwardBackward::c_Forward) {
          return EForwardBackward::c_Forward;
        } else if (fromIsCoaligned == EForwardBackward::c_Backward and
                   toIsCoaligned == EForwardBackward::c_Backward) {
          return EForwardBackward::c_Backward;
        } else {
          return EForwardBackward::c_Invalid;
        }
      }

      /// Setter for the three dimensional trajectory
      void setTrajectory3D(const CDCTrajectory3D& trajectory3D) const
      { m_trajectory3D = trajectory3D; }

      /// Getter for the three dimensional trajectory.
      CDCTrajectory3D& getTrajectory3D() const
      { return m_trajectory3D; }

      /// Getter for the two dimensional projection of the common three dimensional trajectory.
      CDCTrajectory2D getTrajectory2D() const
      { return getTrajectory3D().getTrajectory2D(); }

      /// Getter for the sz projection of the common three dimensional trajectory.
      CDCTrajectorySZ getTrajectorySZ() const
      { return getTrajectory3D().getTrajectorySZ(); }

      /// Invalides the currently stored trajectory information.
      void clearTrajectory3D() const
      { getTrajectory3D().clear(); }


      /// Unsets the masked flag of the segment triple's automaton cell and of the three contained segments.
      void unsetAndForwardMaskedFlag() const
      {
        getAutomatonCell().unsetMaskedFlag();
        const bool toHits = true;
        getFromSegment()->unsetAndForwardMaskedFlag(toHits);
        getToSegment()->unsetAndForwardMaskedFlag(toHits);
      }

      /// Sets the masked flag of the segment triple's automaton cell and of the three contained segments.
      void setAndForwardMaskedFlag() const
      {
        getAutomatonCell().setMaskedFlag();
        const bool toHits = true;
        getFromSegment()->setAndForwardMaskedFlag(toHits);
        getToSegment()->setAndForwardMaskedFlag(toHits);
      }

      /// If one of the contained segments is marked as masked this segment triple is set be masked as well.
      void receiveMaskedFlag() const
      {
        const bool fromHits = true;
        getFromSegment()->receiveMaskedFlag(fromHits);
        getToSegment()->receiveMaskedFlag(fromHits);

        if (getFromSegment()->getAutomatonCell().hasMaskedFlag() or
            getToSegment()->getAutomatonCell().hasMaskedFlag()) {
          getAutomatonCell().setMaskedFlag();
        }
      }

      /// Mutable getter for the automaton cell.
      AutomatonCell& getAutomatonCell() const
      {
        return m_automatonCell;
      }

    private:
      /// Reference to the from segment
      const CDCSegment2D* m_fromSegment;

      /// Reference to the to segment
      const CDCSegment2D* m_toSegment;

      /// Memory for the common three dimensional trajectory
      mutable CDCTrajectory3D m_trajectory3D;

      /// Automaton cell assoziated with the pair of segments
      mutable AutomatonCell m_automatonCell;
    };
  }
}
