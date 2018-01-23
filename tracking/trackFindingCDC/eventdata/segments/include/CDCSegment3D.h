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

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>

#include <tracking/trackFindingCDC/ca/AutomatonCell.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegment2D;
    class CDCTrajectory2D;

    /// A segment consisting of three dimensional reconstructed hits.
    class CDCSegment3D : public CDCSegment<CDCRecoHit3D> {

    public:
      /// Reconstructs a two dimensional stereo segment by shifting each hit onto the given two dimensional trajectory.
      static CDCSegment3D reconstruct(const CDCSegment2D& segment2D,
                                      const CDCTrajectory2D& trajectory2D);

      /**
       *  Constructs a two dimensional segment by carrying out the stereo ! projection to the wire reference postion.
       *  Note : no fitting information is transported to the resulting segment.
       */
      CDCSegment2D stereoProjectToRef() const;

      /// Mutable getter for the automaton cell.
      AutomatonCell& getAutomatonCell() const
      {
        return m_automatonCell;
      }

      /// Indirection to the automaton cell for easier access to the flags
      AutomatonCell* operator->() const
      {
        return &m_automatonCell;
      }

      /**
       *  Unset the masked flag of the automaton cell of this segment
       *  and of all contained wire hits.
       */
      void unsetAndForwardMaskedFlag(bool toHits = true) const;

      /**
       *  Set the masked flag of the automaton cell of this segment
       *  and forward the masked flag to all contained wire hits.
       */
      void setAndForwardMaskedFlag(bool toHits = true) const;

      /**
       *  Check all contained wire hits if one has the masked flag.
       *  Set the masked flag of this segment in case at least one of
       *  the contained wire hits is flagged as masked.
       */
      void receiveMaskedFlag(bool fromHits = true) const;

      /// Getter for the two dimensional trajectory fitted to the segment
      CDCTrajectory3D& getTrajectory3D() const
      {
        return m_trajectory3D;
      }

      /// Setter for the two dimensional trajectory fitted to the segment
      void setTrajectory3D(const CDCTrajectory3D& trajectory3D) const
      {
        m_trajectory3D = trajectory3D;
      }

    private:
      /**
       *  Memory for the automaton cell.
       *  It is declared mutable because it can vary
       *  rather freely despite of the hit content might be required fixed.
       */
      mutable AutomatonCell m_automatonCell;

      /// Memory for the three dimensional trajectory fitted to this segment
      mutable CDCTrajectory3D m_trajectory3D;
    };
  }
}
