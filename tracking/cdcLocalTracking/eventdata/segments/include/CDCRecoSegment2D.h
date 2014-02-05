/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCRECOSEGMENT2D_H_
#define CDCRECOSEGMENT2D_H_

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <tracking/cdcLocalTracking/eventdata/collections/CDCRecoHit2DVector.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// A segment consisting of two dimensional reconsturcted hits
    class CDCRecoSegment2D : public CDCRecoHit2DVector {
    public:

      /// Default constructor for ROOT compatibility.
      CDCRecoSegment2D() {;}

      /// Empty deconstructor
      ~CDCRecoSegment2D() {;}

      ///Implements the standard swap idiom
      friend void swap(CDCRecoSegment2D& lhs, CDCRecoSegment2D& rhs) {
        SortableVector<CDCRecoHit2D>& rawLHS = lhs;
        SortableVector<CDCRecoHit2D>& rawRHS = rhs;
        rawLHS.swap(rawRHS);
        B2DEBUG(200, "CDCRecoSegment::swap");
      }

      /// Getter for the automaton cell.
      AutomatonCell& getAutomatonCell() { return m_automatonCell; }

      /// Constant getter for the automaton cell.
      AutomatonCell& getAutomatonCell() const { return m_automatonCell; }

      /// Getter for the two dimensional trajectory fitted to the segment
      CDCTrajectory2D& getTrajectory2D() const
      { return m_trajectory2D; }

      /// Setter for the two dimensional trajectory fitted to the segment
      void setTrajectory2D(const CDCTrajectory2D& trajectory2D) const
      { m_trajectory2D =  trajectory2D; }

    private:
      mutable AutomatonCell m_automatonCell; ///< Memory for the automaton cell. It is declared mutable because it can vary rather freely despite of the hit content might be required fixed
      mutable CDCTrajectory2D m_trajectory2D; ///< Memory for the two dimensional trajectory fitted to this segment

    private:
      /// ROOT Macro to make CDCRecoSegment2D a ROOT class.
      ClassDefInCDCLocalTracking(CDCRecoSegment2D, 1);



    }; //end class CDCRecoSegment2D

  } // end namespace CDCLocalTracking
} // end namespace Belle2
#endif // CDCRECOSEGMENT2D_H_
