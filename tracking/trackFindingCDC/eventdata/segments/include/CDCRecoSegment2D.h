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

#include <tracking/trackFindingCDC/typedefs/BasicTypes.h>

#include <tracking/trackFindingCDC/eventdata/collections/CDCRecoHit2DVector.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitSegment.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRLWireHitSegment.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCTangentSegment.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCFacetSegment.h>

namespace genfit {
  class TrackCand;
}

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A segment consisting of two dimensional reconsturcted hits
    class CDCRecoSegment2D : public CDCRecoHit2DVector {
    public:

      /// Default constructor for ROOT compatibility.
      CDCRecoSegment2D() {;}

      /// Empty deconstructor
      ~CDCRecoSegment2D() {;}


      /// Averages the reconstructed positions from hits that overlap in adjacent tangents in the given tangent segment
      static CDCRecoSegment2D condense(const CDCTangentSegment& tangentSegment);

      /// Averages the reconstructed positions from hits that overlap in adjacent tangents in the given tangent path
      static CDCRecoSegment2D condense(const std::vector<const Belle2::TrackFindingCDC::CDCTangent* >& tangentPath);

      /// Averages the reconstructed positions from hits that overlap in adjacent facets in the given facet segment.
      static CDCRecoSegment2D condense(const CDCFacetSegment& facetSegment);

      /// Averages the reconstructed positions from hits that overlap in adjacent facet in the given facet path.
      static CDCRecoSegment2D condense(const std::vector<const Belle2::TrackFindingCDC::CDCFacet* >& facetPath);

      /** Flattens a series of segments to one segment.
       *  Does not copy any fit.
       */
      static CDCRecoSegment2D condense(const std::vector<const Belle2::TrackFindingCDC::CDCRecoSegment2D*>& segmentPath);

      /** Reconstruct from wire hits with attached right left passage hypotheses
       *  by constructing tangents between adjacent hits pairs and averaging the reconstucted position.
       */
      static CDCRecoSegment2D reconstructUsingTangents(const CDCRLWireHitSegment& rlWireHitSegment);

      /** Reconstruct from wire hits with attached right left passage hypotheses
       *  by constructing facets between adjacent hits triples and averaging the reconstucted position.
       */
      static CDCRecoSegment2D reconstructUsingFacets(const CDCRLWireHitSegment& rlWireHitSegment);

      ///Implements the standard swap idiom
      friend void swap(CDCRecoSegment2D& lhs, CDCRecoSegment2D& rhs)
      {
        SortableVector<CDCRecoHit2D>& rawLHS = lhs;
        SortableVector<CDCRecoHit2D>& rawRHS = rhs;
        rawLHS.swap(rawRHS);
        std::swap(lhs.m_trajectory2D, rhs.m_trajectory2D);
        std::swap(lhs.m_automatonCell, rhs.m_automatonCell);
        B2DEBUG(200, "CDCRecoSegment::swap");
      }

      /// Defines segments and superlayers to be coaligned.
      friend bool operator<(const CDCRecoSegment2D& segment, const CDCWireSuperLayer& wireSuperLayer)
      { return segment.getISuperLayer() < wireSuperLayer.getISuperLayer(); }

      /// Defines segments and superlayers to be coaligned.
      friend bool operator<(const CDCWireSuperLayer& wireSuperLayer, const CDCRecoSegment2D& segment)
      { return wireSuperLayer.getISuperLayer() < segment.getISuperLayer(); }

      /// Allow automatic taking of the address.
      /** Essentially pointers to (lvalue) objects is a subclass of the object itself.
       *  This method activally exposes this inheritance to be able to write algorithms that work for objects and poiinters alike without code duplication.
       *  \note Once reference qualifiers become available use an & after the trailing const to constrain the cast to lvalues.*/
      operator const Belle2::TrackFindingCDC::CDCRecoSegment2D* () const
      { return this; }

      /// Getter for the vector of wires the hits of this segment are based on in the same order
      std::vector<const Belle2::TrackFindingCDC::CDCWire*> getWireSegment() const
      {
        std::vector<const Belle2::TrackFindingCDC::CDCWire*> wireSegment;
        for (const CDCRecoHit2D& recoHit2D : *this) {
          wireSegment.push_back(&(recoHit2D.getWire()));
        }
        return wireSegment;
      }

      /// Getter for the vector of wires the hits of this segment are based on in the same order
      CDCWireHitSegment getWireHitSegment() const
      {
        CDCWireHitSegment wireHitSegment;
        for (const CDCRecoHit2D& recoHit2D : *this) {
          wireHitSegment.push_back(&(recoHit2D.getWireHit()));
        }
        return wireHitSegment;
      }

      /// Getter for the vector of wires the hits of this segment are based on in the same order
      CDCRLWireHitSegment getRLWireHitSegment() const
      {
        CDCRLWireHitSegment rlWireHitSegment;
        for (const CDCRecoHit2D& recoHit2D : *this) {
          rlWireHitSegment.push_back(&(recoHit2D.getRLWireHit()));
        }
        return rlWireHitSegment;
      }

      /// Fill the hit content of this segment into a genfit::TrackCand. Return true, if the trajectory information is valid, false otherwise.
      bool fillInto(genfit::TrackCand& gfTrackCand) const;

      /// Makes a copy of the segment with the reversed hits in the opposite order.
      CDCRecoSegment2D reversed() const
      {

        CDCRecoSegment2D reverseSegment;
        reverseSegment.reserve(size());
        for (const CDCRecoHit2D& recohit : reverseRange()) {
          reverseSegment.push_back(recohit.reversed());
        }

        reverseSegment.setTrajectory2D(getTrajectory2D().reversed());
        reverseSegment.m_automatonCell = m_automatonCell;
        return reverseSegment;
      }

      /// Reverses the order of hits and their right left passage hypotheses inplace
      void reverse()
      {
        // Reverse the trajectory
        m_trajectory2D.reverse();

        // Reverse the left right passage hypotheses
        for (CDCRecoHit2D& recoHit2D : *this) {
          recoHit2D.reverse();
        }
        // Reverse the arrangement of hits.
        std::reverse(begin(), end());
      }

      /// Getter for the automaton cell.
      AutomatonCell& getAutomatonCell() { return m_automatonCell; }

      /// Constant getter for the automaton cell.
      AutomatonCell& getAutomatonCell() const { return m_automatonCell; }

      /// Set the taken flag of the automaton cell of this segment and forward the taken flag to all contained wire hits.
      void setAndForwardTakenFlag() const
      {
        getAutomatonCell().setTakenFlag();
        for (const CDCRecoHit2D& recoHit2D : *this) {
          const CDCWireHit& wireHit = recoHit2D.getWireHit();

          wireHit.getAutomatonCell().setTakenFlag();

        }
      }

      /// Check all contained wire hits if one has the taken flag. Set the taken flag of this segment in case at least one of the contained wire hits is flagged as taken.
      void receiveTakenFlag() const
      {
        for (const CDCRecoHit2D& recoHit2D : *this) {
          const CDCWireHit& wireHit = recoHit2D.getWireHit();

          if (wireHit.getAutomatonCell().hasTakenFlag()) {
            getAutomatonCell().setTakenFlag();
            return;
          }

        }

      }

      /// Indicates if two segments share some wire hits
      bool sharesWireHit(const CDCRecoSegment2D& segment) const
      {
        for (const CDCRecoHit2D& recoHit : segment) {
          const CDCWireHit& wireHit = recoHit.getWireHit();
          if (hasWireHit(wireHit)) {
            return true;
          }
        }
        return false;
      }

      /// Getter for the two dimensional trajectory fitted to the segment
      CDCTrajectory2D& getTrajectory2D() const
      { return m_trajectory2D; }

      /// Setter for the two dimensional trajectory fitted to the segment
      void setTrajectory2D(const CDCTrajectory2D& trajectory2D) const
      { m_trajectory2D =  trajectory2D; }

      /// Reconstructs a central point on the trajectory
      Vector2D getCenterRecoPos2D() const
      { return getTrajectory2D().getClosest(getCenterOfMass2D()); }

      /// Reconstructs a point on the attached trajectory close to the front of the segment
      Vector2D getFrontRecoPos2D() const
      { return front().getFrontRecoPos2D(getTrajectory2D()); }

      /// Reconstructs a point on the given trajectory close to the front of the segment
      Vector2D getFrontRecoPos2D(const CDCTrajectory2D& trajectory2D) const
      { return front().getFrontRecoPos2D(trajectory2D); }


      /// Reconstructs a point on the attached trajectory close to the back of the segment
      Vector2D getBackRecoPos2D() const
      { return back().getBackRecoPos2D(getTrajectory2D()); }

      /// Reconstructs a point on the attached trajectory close to the back of the segment
      Vector2D getBackRecoPos2D(const CDCTrajectory2D& trajectory2D) const
      { return back().getBackRecoPos2D(trajectory2D); }


      /// Getter for the global super cluster id.
      const size_t& getISuperCluster() const
      { return m_iSuperCluster; }

      /// Setter for the globale super cluster id.
      void setISuperCluster(const size_t& iSuperCluster)
      { m_iSuperCluster = iSuperCluster; }

    private:
      mutable AutomatonCell
      m_automatonCell; ///< Memory for the automaton cell. It is declared mutable because it can vary rather freely despite of the hit content might be required fixed
      mutable CDCTrajectory2D m_trajectory2D; ///< Memory for the two dimensional trajectory fitted to this segment

      /// Memory for the global super cluster id.
      size_t m_iSuperCluster;

    private:



    }; //end class CDCRecoSegment2D

  } // end namespace TrackFindingCDC
} // end namespace Belle2
