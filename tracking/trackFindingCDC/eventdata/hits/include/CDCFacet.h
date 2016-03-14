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
#include <tracking/trackFindingCDC/geometry/ParameterLine2D.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit2D.h>

#include <tracking/trackFindingCDC/ca/AutomatonCell.h>
#include <tracking/trackFindingCDC/numerics/BasicTypes.h>

#include <utility>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class representing a triple of neighboring wire hits
    class CDCFacet : public CDCRLWireHitTriple {

    public:
      /// Default constructor for ROOT compatibility.
      CDCFacet();

      /// Constructor taking three oriented wire hits
      CDCFacet(const CDCRLWireHit* startRLWireHit,
               const CDCRLWireHit* middleRLWireHit,
               const CDCRLWireHit* endRLWireHit);

      /// Constructor taking three oriented wire hits and the tangent lines
      CDCFacet(const CDCRLWireHit* startRLWireHit,
               const CDCRLWireHit* middleRLWireHit,
               const CDCRLWireHit* endRLWireHit,
               const ParameterLine2D& startToMiddle,
               const ParameterLine2D& startToEnd = ParameterLine2D(),
               const ParameterLine2D& middleToEnd = ParameterLine2D());

    public:
      /// Constructs the reverse tiple from this one - this ignores the tangent lines
      CDCFacet reversed() const;

    public:

      /// Access the object methods and methods from a pointer in the same way.
      /** In situations where the type is not known to be a pointer or a reference there is no way to tell \n
       *  if one should use the dot '.' or operator '->' for method look up. \n
       *  So this function defines the -> operator for the object. \n
       *  No matter you have a pointer or an object access is given with '->'*/
      const CDCFacet* operator->() const
      { return this; }

      /// Allow automatic taking of the address.
      /** Essentially pointers to (lvalue) objects is a subclass of the object itself.
       *  This method activally exposes this inheritance to be able to write algorithms
       *  that work for objects and poiinters alike without code duplication.
       *  \note Once reference qualifiers become available use an & after the trailing const to constrain the cast to lvalues.*/
      operator const Belle2::TrackFindingCDC::CDCFacet* () const
      { return this; }

      /// Getter for the tangential line from the first to the second hit
      const ParameterLine2D& getStartToMiddleLine() const
      { return m_startToMiddle; }

      /// Getter for the tangential line from the first to the third hit
      const ParameterLine2D& getStartToEndLine() const
      { return m_startToEnd; }

      /// Getter for the tangential line from the second to the third hit
      const ParameterLine2D& getMiddleToEndLine() const
      { return m_middleToEnd; }



      /// Construct and stores the three tangential lines corresponding to the three pairs of wire hits.
      void adjustLines() const;



      /// Getter for the recostructed position at the first hit averaged from the two touching points of the tangential lines
      Vector2D getStartRecoPos2D() const
      { return Vector2D::average(getStartToMiddleLine().at(0), getStartToEndLine().at(0)); }

      /// Getter for the recostructed position at the second hit averaged from the two touching points of the tangential lines
      Vector2D getMiddleRecoPos2D() const
      { return Vector2D::average(getStartToMiddleLine().at(1), getMiddleToEndLine().at(0)); }

      /// Getter for the recostructed position at the third hit averaged from the two touching points of the tangential lines
      Vector2D getEndRecoPos2D() const
      { return Vector2D::average(getStartToEndLine().at(1), getMiddleToEndLine().at(1)); }

      /// Getter for the recostructed position including the first hit averaged from the two touching points of the tangential lines
      CDCRecoHit2D getStartRecoHit2D() const
      { return CDCRecoHit2D::fromRecoPos2D(getStartRLWireHit(), getStartRecoPos2D()); }

      /// Getter for the recostructed position including the second hit averaged from the two touching points of the tangential lines
      CDCRecoHit2D getMiddleRecoHit2D() const
      { return CDCRecoHit2D::fromRecoPos2D(getMiddleRLWireHit(), getMiddleRecoPos2D()); }

      /// Getter for the recostructed position including the third hit averaged from the two touching points of the tangential lines
      CDCRecoHit2D getEndRecoHit2D() const
      { return CDCRecoHit2D::fromRecoPos2D(getEndRLWireHit(), getEndRecoPos2D()); }


      /// Getter for the tangential line including the hits from the first to the second hit
      CDCTangent getStartToMiddle() const
      { return CDCTangent(&(getStartRLWireHit()), &(getEndRLWireHit()), getStartToMiddleLine()); }

      /// Getter for the tangential line including the hits from the first to the third hit
      CDCTangent getStartToEnd() const
      { return CDCTangent(&(getStartRLWireHit()), &(getEndRLWireHit()), getStartToEndLine()); }

      /// Getter for the tangential line including the hits from the second to the third hit
      CDCTangent getMiddleToEnd() const
      { return CDCTangent(&(getMiddleRLWireHit()), &(getEndRLWireHit()), getMiddleToEndLine()); }


      /// Unset the masked flag of the facet's automaton cell and of the three contained wire hits
      void unsetAndForwardMaskedFlag() const
      {
        getAutomatonCell().unsetMaskedFlag();
        getStartWireHit().getAutomatonCell().unsetMaskedFlag();
        getMiddleWireHit().getAutomatonCell().unsetMaskedFlag();
        getEndWireHit().getAutomatonCell().unsetMaskedFlag();
      }

      /// Sets the masked flag of the facet's automaton cell and of the three contained wire hits
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

      /// Getter for the automaton cell.
      AutomatonCell& getAutomatonCell()
      { return m_automatonCell; }

      /// Constant getter for the automaton cell.
      const AutomatonCell& getAutomatonCell() const
      { return m_automatonCell; }

    private:
      /// Memory for the tangential line between first and second hit
      mutable ParameterLine2D m_startToMiddle;

      /// Memory for the tangential line between first and third hit
      mutable ParameterLine2D m_startToEnd;

      /// Memory for the tangential line between second and third hit
      mutable ParameterLine2D m_middleToEnd;

      /// Memory for the cellular automaton cell assoziated with the facet.
      AutomatonCell m_automatonCell;

    }; //class

  } // namespace TrackFindingCDC
} // namespace Belle2
