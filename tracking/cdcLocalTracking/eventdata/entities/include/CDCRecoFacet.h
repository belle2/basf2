/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCRECOFACET_H
#define CDCRECOFACET_H

#include <utility>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/algorithms/AutomatonCell.h>

#include "CDCRLWireHitTriple.h"

#include "CDCRecoHit2D.h"
#include "CDCRecoTangent.h"


namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class representing a triple of neighboring wire hits
    class CDCRecoFacet : public CDCRLWireHitTriple {

    public:
      /// Default constructor for ROOT compatibility.
      CDCRecoFacet();

      /// Constructor taking three oriented wire hits
      CDCRecoFacet(
        const CDCRLWireHit* startRLWireHit,
        const CDCRLWireHit* middleRLWireHit,
        const CDCRLWireHit* endRLWireHit
      );

      /// Constructor taking three oriented wire hits and the tangent lines
      CDCRecoFacet(
        const CDCRLWireHit* startRLWireHit,
        const CDCRLWireHit* middleRLWireHit,
        const CDCRLWireHit* endRLWireHit,
        const ParameterLine2D& startToMiddle,
        const ParameterLine2D& startToEnd = ParameterLine2D(),
        const ParameterLine2D& middleToEnd = ParameterLine2D()
      );

      /// Empty destructor
      ~CDCRecoFacet() {;}

    public:
      /// Constructs the reverse tiple from this one - this ignores the tangent lines
      CDCRecoFacet reversed() const;

    public:

      /// Access the object methods and methods from a pointer in the same way.
      /** In situations where the type is not known to be a pointer or a reference there is no way to tell \n
       *  if one should use the dot '.' or operator '->' for method look up. \n
       *  So this function defines the -> operator for the object. \n
       *  No matter you have a pointer or an object access is given with '->'*/
      const CDCRecoFacet* operator->() const { return this; }


      const ParameterLine2D& getStartToMiddleLine() const
      { return m_startToMiddle; }

      const ParameterLine2D& getStartToEndLine() const
      { return m_startToEnd; }

      const ParameterLine2D& getMiddleToEndLine() const
      { return m_middleToEnd; }

      void adjustLines() const;

      Vector2D getStartRefPos2D() const
      { return Vector2D::average(getStartToMiddleLine().at(0), getStartToEndLine().at(0)); }

      Vector2D getMiddleRefPos2D() const
      { return Vector2D::average(getStartToMiddleLine().at(1), getMiddleToEndLine().at(0)); }

      Vector2D getEndRefPos2D() const
      { return Vector2D::average(getStartToEndLine().at(1), getMiddleToEndLine().at(1)); }

      Vector2D getCenterOfMass2D() const
      { return Vector2D::average(getStartRefPos2D(), getMiddleRefPos2D(), getEndRefPos2D()); }



      CDCRecoHit2D getStartRecoHit2D() const
      { return CDCRecoHit2D::fromAbsPos2D(&(getStartRLWireHit()), getStartRefPos2D()); }

      CDCRecoHit2D getMiddleRecoHit2D() const
      { return CDCRecoHit2D::fromAbsPos2D(&(getMiddleRLWireHit()), getMiddleRefPos2D()); }

      CDCRecoHit2D getEndRecoHit2D() const
      { return CDCRecoHit2D::fromAbsPos2D(&(getEndRLWireHit()), getEndRefPos2D()); }

      CDCRecoTangent getStartToMiddle() const
      { return CDCRecoTangent(&(getStartRLWireHit()), &(getMiddleRLWireHit()), getStartToMiddleLine()); }

      CDCRecoTangent getStartToEnd() const
      { return CDCRecoTangent(&(getStartRLWireHit()), &(getEndRLWireHit()), getStartToEndLine()); }

      CDCRecoTangent getMiddleToEnd() const
      { return CDCRecoTangent(&(getMiddleRLWireHit()), &(getEndRLWireHit()), getMiddleToEndLine()); }



      FloatType getStartPerpS(const CDCTrajectory2D& trajectory2D) const
      { return trajectory2D.calcPerpS(getStartRefPos2D()); }

      FloatType getEndPerpS(const CDCTrajectory2D& trajectory2D) const
      { return trajectory2D.calcPerpS(getEndRefPos2D()); }

      FloatType getSquaredDist2D(const CDCTrajectory2D& trajectory2D) const;



      /// Sets the do not use flag of the facet's automaton cell and of the three contained wire hits
      void setDoNotUse() const {
        getAutomatonCell().setFlags(DO_NOT_USE);
        forwardDoNotUse();
      }

      /// Sets the do not use flag of the three contained wire hits
      void forwardDoNotUse() const {
        getStartWireHit().getAutomatonCell().setFlags(DO_NOT_USE);
        getMiddleWireHit().getAutomatonCell().setFlags(DO_NOT_USE);
        getEndWireHit().getAutomatonCell().setFlags(DO_NOT_USE);
      }

      /// If one of the contained wire hits is marked as do not use this facet is set be not usable as well
      void receiveDoNotUse() const {

        if (getStartWireHit().getAutomatonCell().hasAnyFlags(DO_NOT_USE) or
            getMiddleWireHit().getAutomatonCell().hasAnyFlags(DO_NOT_USE) or
            getEndWireHit().getAutomatonCell().hasAnyFlags(DO_NOT_USE)) {

          getAutomatonCell().setFlags(DO_NOT_USE);
        }
      }

      /// Getter for the automaton cell.
      AutomatonCell& getAutomatonCell() { return m_automatonCell; }

      /// Constant getter for the automaton cell.
      const AutomatonCell& getAutomatonCell() const { return m_automatonCell; }

    private:

      mutable ParameterLine2D m_startToMiddle;
      mutable ParameterLine2D m_startToEnd;
      mutable ParameterLine2D m_middleToEnd;

      AutomatonCell m_automatonCell;

      /// ROOT Macro to make CDCRecoFacet a ROOT class.
      ClassDefInCDCLocalTracking(CDCRecoFacet, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCRECOFACET_H
