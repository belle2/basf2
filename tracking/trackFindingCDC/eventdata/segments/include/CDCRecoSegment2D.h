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
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit2D.h>

namespace genfit {
  class TrackCand;
}

namespace Belle2 {
  namespace TrackFindingCDC {

    class CDCFacetSegment;
    class CDCTangentSegment;
    class CDCRLWireHitSegment;
    class CDCWireHitSegment;

    class CDCFacet;
    class CDCTangent;

    /// A segment consisting of two dimensional reconsturcted hits.
    class CDCRecoSegment2D : public CDCSegment<CDCRecoHit2D> {
    public:
      /// Averages the reconstructed positions from hits that overlap
      /// in adjacent tangents in the given tangent segment.
      static CDCRecoSegment2D condense(const CDCTangentSegment& tangentSegment);

      /// Averages the reconstructed positions from hits that overlap
      /// in adjacent tangents in the given tangent path.
      static CDCRecoSegment2D condense(const std::vector<const Belle2::TrackFindingCDC::CDCTangent* >& tangentPath);

      /// Averages the reconstructed positions from hits that overlap
      /// in adjacent facets in the given facet segment.
      static CDCRecoSegment2D condense(const CDCFacetSegment& facetSegment);

      /// Averages the reconstructed positions from hits that overlap
      /// in adjacent facet in the given facet path.
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

      /// Allow automatic taking of the address.
      /** Essentially pointers to (lvalue) objects is a subclass of the object itself.
       *  This method activally exposes this inheritance to be able to write algorithms
       *  that work for objects and poiinters alike without code duplication.*/
      operator const Belle2::TrackFindingCDC::CDCRecoSegment2D* () const&
      { return this; }

      /// Getter for the vector of wires the hits of this segment are based on in the same order.
      std::vector<const Belle2::TrackFindingCDC::CDCWire*> getWireSegment() const;

      /// Getter for the vector of wires the hits of this segment are based on in the same order.
      CDCWireHitSegment getWireHitSegment() const;

      /** Fill the hit content of this segment into a genfit::TrackCand.
       *  @return true, if the trajectory information is valid, false otherwise.
       */
      bool fillInto(genfit::TrackCand& gfTrackCand) const;

      /// Makes a copy of the segment with the reversed hits in the opposite order.
      CDCRecoSegment2D reversed() const;

      /// Reverses the order of hits and their right left passage hypotheses inplace.
      void reverse();

      /// Getter for the automaton cell.
      AutomatonCell& getAutomatonCell()
      { return m_automatonCell; }

      /// Constant getter for the automaton cell.
      const AutomatonCell& getAutomatonCell() const
      { return m_automatonCell; }

      /// Unset the masked flag of the automaton cell of this segment
      /// and of all contained wire hits.
      void unsetAndForwardMaskedFlag() const;

      /// Set the masked flag of the automaton cell of this segment
      /// and forward the masked flag to all contained wire hits.
      void setAndForwardMaskedFlag() const;

      /// Check all contained wire hits if one has the masked flag.
      /** Set the masked flag of this segment in case at least one of
       *  the contained wire hits is flagged as masked.*/
      void receiveMaskedFlag() const;

      /// Getter for the global super cluster id.
      int getISuperCluster() const
      { return m_iSuperCluster; }

      /// Setter for the globale super cluster id.
      void setISuperCluster(int iSuperCluster)
      { m_iSuperCluster = iSuperCluster; }

      /// Setter for the super cluster id based on the hit content
      void receiveISuperCluster()
      {
        auto getISuperClusterOfHit = [](const CDCRecoHit2D & recoHit2d) -> int
        { return recoHit2d.getWireHit().getISuperCluster(); };
        int iSuperCluster = common(*this, getISuperClusterOfHit, -1);
        setISuperCluster(iSuperCluster);
      }

      /// Returns false, if there is one hit in the range which does not have a taken flag.
      bool isFullyTaken() const;

    private:
      /** Memory for the automaton cell.
       *  It is declared mutable because it can vary
       *  rather freely despite of the hit content might be required fixed.
       */
      AutomatonCell m_automatonCell;

      /// Memory for the global super cluster id.
      int m_iSuperCluster = -1;
    }; //end class CDCRecoSegment2D
  } // end namespace TrackFindingCDC
} // end namespace Belle2
