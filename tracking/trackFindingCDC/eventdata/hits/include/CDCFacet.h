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
#include <tracking/trackFindingCDC/eventdata/hits/CDCTangent.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit2D.h>

#include <tracking/trackFindingCDC/geometry/UncertainParameterLine2D.h>
#include <tracking/trackFindingCDC/geometry/ParameterLine2D.h>

#include <tracking/trackFindingCDC/ca/AutomatonCell.h>

#include <utility>

namespace Belle2 {
  namespace TrackFindingCDC {

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
      { return m_fitLine; }

      /// Setter for the contained line fit information.
      void setFitLine(const UncertainParameterLine2D& fitLine) const
      { m_fitLine = fitLine; }

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
      Vector2D getStartRecoPos2D() const
      { return getFitLine()->closest(getStartWire().getRefPos2D()); }

      /// Getter for the reconstructed position at the second hit on the fit line
      Vector2D getMiddleRecoPos2D() const
      { return getFitLine()->closest(getMiddleWire().getRefPos2D()); }

      /// Getter for the reconstructed position at the third hit on the fit line
      Vector2D getEndRecoPos2D() const
      { return getFitLine()->closest(getEndWire().getRefPos2D()); }

      /// Getter for the first reconstucted hit
      CDCRecoHit2D getStartRecoHit2D() const
      { return CDCRecoHit2D::fromRecoPos2D(getStartRLWireHit(), getStartRecoPos2D()); }

      /// Getter for the second reconstucted hit
      CDCRecoHit2D getMiddleRecoHit2D() const
      { return CDCRecoHit2D::fromRecoPos2D(getMiddleRLWireHit(), getMiddleRecoPos2D()); }

      /// Getter for the third reconstucted hit
      CDCRecoHit2D getEndRecoHit2D() const
      { return CDCRecoHit2D::fromRecoPos2D(getEndRLWireHit(), getEndRecoPos2D()); }

      /// Getter for the tangential line including the hits from the first to the second hit.
      CDCTangent getStartToMiddle() const
      { return CDCTangent(getStartRLWireHit(), getEndRLWireHit(), getStartToMiddleLine()); }

      /// Getter for the tangential line including the hits from the first to the third hit.
      CDCTangent getStartToEnd() const
      { return CDCTangent(getStartRLWireHit(), getEndRLWireHit(), getStartToEndLine()); }

      /// Getter for the tangential line including the hits from the second to the third hit.
      CDCTangent getMiddleToEnd() const
      { return CDCTangent(getMiddleRLWireHit(), getEndRLWireHit(), getMiddleToEndLine()); }

      /// Unset the masked flag of the facet's automaton cell and of the three contained wire hits.
      void unsetAndForwardMaskedFlag() const
      {
        getAutomatonCell().unsetMaskedFlag();
        getStartWireHit().getAutomatonCell().unsetMaskedFlag();
        getMiddleWireHit().getAutomatonCell().unsetMaskedFlag();
        getEndWireHit().getAutomatonCell().unsetMaskedFlag();
      }

      /// Sets the masked flag of the facet's automaton cell and of the three contained wire hits.
      void setAndForwardMaskedFlag() const
      {
        getAutomatonCell().setMaskedFlag();
        getStartWireHit().getAutomatonCell().setMaskedFlag();
        getMiddleWireHit().getAutomatonCell().setMaskedFlag();
        getEndWireHit().getAutomatonCell().setMaskedFlag();
      }

      /// If one of the contained wire hits is marked as masked this facet is set be masked as well.
      void receiveMaskedFlag() const
      {
        if (getStartWireHit().getAutomatonCell().hasMaskedFlag() or
            getMiddleWireHit().getAutomatonCell().hasMaskedFlag() or
            getEndWireHit().getAutomatonCell().hasMaskedFlag()) {

          getAutomatonCell().setMaskedFlag();
        }
      }

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
