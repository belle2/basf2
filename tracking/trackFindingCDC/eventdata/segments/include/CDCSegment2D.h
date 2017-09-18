/*!************************************************************************
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

#include <tracking/trackFindingCDC/ca/AutomatonCell.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    class CDCFacetSegment;
    class CDCTangentSegment;
    class CDCRLWireHitSegment;
    class CDCWireHitSegment;

    class CDCFacet;
    class CDCTangent;

    class CDCWire;

    template<class T, class S> class WeightedRelation;
    template<class T, class S> class Relation;

    //! A reconstructed sequence of two dimensional hits in one super layer
    class CDCSegment2D : public CDCSegment<CDCRecoHit2D> {
    public:
      /*!
       *  Averages the reconstructed positions from hits that overlap
       *  in adjacent tangents in the given tangent segment.
       */
      static CDCSegment2D condense(const CDCTangentSegment& tangentSegment);

      /*!
       *  Averages the reconstructed positions from hits that overlap
       *  in adjacent tangents in the given tangent path.
       */
      static CDCSegment2D condense(const std::vector<const CDCTangent*>& tangentPath);

      /*!
       *  Averages the reconstructed positions from hits that overlap
       *  in adjacent facets in the given facet segment.
       */
      static CDCSegment2D condense(const CDCFacetSegment& facetSegment);

      /*!
       *  Averages the reconstructed positions from hits that overlap
       *  in adjacent facet in the given facet path.
       */
      static CDCSegment2D condense(const std::vector<const CDCFacet*>& facetPath);

      /*!
       *  Flattens a series of segments to one segment.
       *  Does not copy any fit.
       */
      static CDCSegment2D condense(const std::vector<const CDCSegment2D*>& segmentPath);

      /*!
       *  Reconstruct from wire hits with attached right left passage hypotheses
       *  by constructing tangents between adjacent hits pairs and averaging the reconstucted position.
       */
      static CDCSegment2D reconstructUsingTangents(const CDCRLWireHitSegment& rlWireHitSegment);

      /*!
       *  Reconstruct from wire hits with attached right left passage hypotheses
       *  by constructing facets between adjacent hits triples and averaging the reconstucted position.
       */
      static CDCSegment2D reconstructUsingFacets(const CDCRLWireHitSegment& rlWireHitSegment);

      //! Comparision of segments up to the super cluster id keeping them close together on sort
      bool operator<(const CDCSegment2D& segment2D) const;

      //! Helper constructor to create a relation in python
      Relation<const CDCSegment2D, const CDCSegment2D>
      makeRelation(const CDCSegment2D* segment) const;

      //! Helper constructor to create a relation in python
      WeightedRelation<const CDCSegment2D, const CDCSegment2D>
      makeWeightedRelation(double weight, const CDCSegment2D* segment) const;

      //! Getter for the vector of wires the hits of this segment are based on in the same order.
      std::vector<const CDCWire*> getWireSegment() const;

      //! Getter for the vector of the wire hits of this segment are based on in the same order.
      CDCWireHitSegment getWireHitSegment() const;

      //! Getter for the vector of right left oriented the hits of this segment.
      CDCRLWireHitSegment getRLWireHitSegment() const;

      //! Getter for the alias version of the segment - fit not copied.
      CDCSegment2D getAlias() const;

      //! Getter for the number of changes in the right left passage in the segment
      int getNRLSwitches() const;

      //! Getter for the sum of right left information relative to the size.
      double getRLAsymmetry() const;

      //! Makes a copy of the segment with the reversed hits in the opposite order.
      CDCSegment2D reversed() const;

      //! Reverses the order of hits and their right left passage hypotheses inplace.
      void reverse();

      //! Mutable getter for the automaton cell.
      AutomatonCell& getAutomatonCell() const
      {
        return m_automatonCell;
      }

      //! Indirection to the automaton cell for easier access to the flags
      AutomatonCell* operator->() const
      {
        return &m_automatonCell;
      }

      /*!
       *  Unset the masked flag of the automaton cell of this segment
       *  and of all contained wire hits.
       */
      void unsetAndForwardMaskedFlag(bool toHits = false) const;

      /*!
       *  Set the masked flag of the automaton cell of this segment
       *  and forward the masked flag to all contained wire hits.
       */
      void setAndForwardMaskedFlag(bool toHits = false) const;

      /*!
       *  Check all contained wire hits if one has the masked flag.
       *  Set the masked flag of this segment in case at least one of
       *  the contained wire hits is flagged as masked.
       */
      void receiveMaskedFlag(bool fromHits = false) const;

      //! Getter for the global super cluster id.
      int getISuperCluster() const
      {
        return m_iSuperCluster;
      }

      //! Setter for the globale super cluster id.
      void setISuperCluster(int iSuperCluster) const
      {
        m_iSuperCluster = iSuperCluster;
      }

      //! Setter for the super cluster id based on the hit content
      void receiveISuperCluster() const;

      //! Returns false, if there are more than N hits in the range which does not have a taken flag.
      bool isFullyTaken(unsigned int maxNotTaken = 0) const;

    private:
      /*!
       *  Memory for the automaton cell.
       *  It is declared mutable because it can vary
       *  rather freely despite of the hit content might be required fixed.
       */
      mutable AutomatonCell m_automatonCell;

      //! Memory for the global super cluster id.
      mutable int m_iSuperCluster = -1;
    };
  }
}
