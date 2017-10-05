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

#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHitTriple.h>
#include <tracking/trackFindingCDC/geometry/UncertainParameterLine2D.h>

#include <tracking/trackFindingCDC/ca/AutomatonCell.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTangent;
    class CDCRecoHit2D;
    class CDCRLWireHit;
    class ParameterLine2D;
    class Vector2D;

    /**
     * Class representing a triple of neighboring oriented wire with additional trajectory information.
     *
     * As trajectory information a single parameter line from a fit is used
     */
    class CDCFacet : public CDCRLWireHitTriple {
    public:
      /// Default constructor for ROOT
      CDCFacet() = default;

      /// Constructor taking three oriented wire hits.
      CDCFacet(const CDCRLWireHit& startRLWireHit,
               const CDCRLWireHit& middleRLWireHit,
               const CDCRLWireHit& endRLWireHit);

      /// Constructor taking three oriented wire hits and the fit line.
      CDCFacet(const CDCRLWireHit& startRLWireHit,
               const CDCRLWireHit& middleRLWireHit,
               const CDCRLWireHit& endRLWireHit,
               const UncertainParameterLine2D& fitLine);

      /// Reverses the facet inplace including the fit line.
      void reverse();

      /// Constructs the reverse tiple from this one.
      CDCFacet reversed() const;

      /// Adjusts the contained fit line to touch such that it touches the first and third hit.
      void adjustFitLine() const;

      /// Clear all information in the fit.
      void invalidateFitLine();

      /// Getter for the contained line fit information.
      const UncertainParameterLine2D& getFitLine() const
      {
        return m_fitLine;
      }

      /// Setter for the contained line fit information.
      void setFitLine(const UncertainParameterLine2D& fitLine) const
      {
        m_fitLine = fitLine;
      }

      /**
       *  Getter for the tangential line from the first to the second hit.
       *  The line is computed as touching the first and second drift circle
       *  assuming the stored right left passage information.
       */
      ParameterLine2D getStartToMiddleLine() const;

      /**
       *  Getter for the tangential line from the first to the third hit.
       *  The line is computed as touching the first and third drift circle
       *  assuming the stored right left passage information
       */
      ParameterLine2D getStartToEndLine() const;

      /**
       *  Getter for the tangential line from the second to the third hit.
       *  The line is computed as touching the second and third drift circle
       *  assuming the stored right left passage information
       */
      ParameterLine2D getMiddleToEndLine() const;

      /// Getter for the reconstructed position at the first hit on the fit line
      Vector2D getStartRecoPos2D() const;

      /// Getter for the reconstructed position at the second hit on the fit line
      Vector2D getMiddleRecoPos2D() const;

      /// Getter for the reconstructed position at the third hit on the fit line
      Vector2D getEndRecoPos2D() const;

      /// Getter for the first reconstucted hit
      CDCRecoHit2D getStartRecoHit2D() const;

      /// Getter for the second reconstucted hit
      CDCRecoHit2D getMiddleRecoHit2D() const;

      /// Getter for the third reconstucted hit
      CDCRecoHit2D getEndRecoHit2D() const;

      /// Getter for the tangential line including the hits from the first to the second hit.
      CDCTangent getStartToMiddle() const;

      /// Getter for the tangential line including the hits from the first to the third hit.
      CDCTangent getStartToEnd() const;

      /// Getter for the tangential line including the hits from the second to the third hit.
      CDCTangent getMiddleToEnd() const;

      /// Unset the masked flag of the facet's automaton cell and of the three contained wire hits.
      void unsetAndForwardMaskedFlag() const;

      /// Sets the masked flag of the facet's automaton cell and of the three contained wire hits.
      void setAndForwardMaskedFlag() const;

      /// If one of the contained wire hits is marked as masked this facet is set be masked as well.
      void receiveMaskedFlag() const;

      /// Mutable getter for the automaton cell.
      AutomatonCell& getAutomatonCell() const
      {
        return m_automatonCell;
      }

    private:
      /// Memory for a line fit to the three contained hits
      mutable UncertainParameterLine2D m_fitLine;

      /// Memory for the cellular automaton cell assoziated with the facet.
      mutable AutomatonCell m_automatonCell;
    };
  }
}
