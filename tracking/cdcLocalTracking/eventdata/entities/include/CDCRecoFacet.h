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


      /// Average of the three reconstructed positions.
      Vector2D getCenterOfMass2D() const
      { return Vector2D::average(getStartRecoPos2D(), getMiddleRecoPos2D(), getEndRecoPos2D()); }


      /// Getter for the recostructed position including the first hit averaged from the two touching points of the tangential lines
      CDCRecoHit2D getStartRecoHit2D() const
      { return CDCRecoHit2D::fromAbsPos2D(&(getStartRLWireHit()), getStartRecoPos2D()); }

      /// Getter for the recostructed position including the second hit averaged from the two touching points of the tangential lines
      CDCRecoHit2D getMiddleRecoHit2D() const
      { return CDCRecoHit2D::fromAbsPos2D(&(getMiddleRLWireHit()), getMiddleRecoPos2D()); }

      /// Getter for the recostructed position including the third hit averaged from the two touching points of the tangential lines
      CDCRecoHit2D getEndRecoHit2D() const
      { return CDCRecoHit2D::fromAbsPos2D(&(getEndRLWireHit()), getEndRecoPos2D()); }


      /// Getter for the tangential line including the hits from the first to the second hit
      CDCRecoTangent getStartToMiddle() const
      { return CDCRecoTangent(&(getStartRLWireHit()), &(getEndRLWireHit()), getStartToMiddleLine()); }

      /// Getter for the tangential line including the hits from the first to the third hit
      CDCRecoTangent getStartToEnd() const
      { return CDCRecoTangent(&(getStartRLWireHit()), &(getEndRLWireHit()), getStartToEndLine()); }

      /// Getter for the tangential line including the hits from the second to the third hit
      CDCRecoTangent getMiddleToEnd() const
      { return CDCRecoTangent(&(getMiddleRLWireHit()), &(getEndRLWireHit()), getMiddleToEndLine()); }


      /// Returns the start reconstucted position projected to the trajectory
      Vector2D getFrontRecoPos2D(const CDCTrajectory2D& trajectory2D) const
      { return trajectory2D.getClosest(getStartRecoPos2D()); }

      /// Returns the end reconstucted position projected to the trajectory
      Vector2D getBackRecoPos2D(const CDCTrajectory2D& trajectory2D) const
      { return trajectory2D.getClosest(getEndRecoPos2D()); }

      /// Estimate the transvers travel distance on the given circle to the reconstructed position at the first hit
      FloatType getFrontPerpS(const CDCTrajectory2D& trajectory2D) const
      { return trajectory2D.calcPerpS(getStartRecoPos2D()); }

      /// Estimate the transvers travel distance on the given circle to the reconstructed position at the last hit
      FloatType getBackPerpS(const CDCTrajectory2D& trajectory2D) const
      { return trajectory2D.calcPerpS(getEndRecoPos2D()); }

      /// Calculates the sum of squared distances of the trajectory to the three hits.
      FloatType getSquaredDist2D(const CDCTrajectory2D& trajectory2D) const;

      /// Sets the do not use flag of the facet's automaton cell and of the three contained wire hits
      void setDoNotUse() const {
        getAutomatonCell().setDoNotUseFlag();
        forwardDoNotUse();
      }

      /// Sets the do not use flag of the three contained wire hits
      void forwardDoNotUse() const {
        getStartWireHit().getAutomatonCell().setDoNotUseFlag();
        getMiddleWireHit().getAutomatonCell().setDoNotUseFlag();
        getEndWireHit().getAutomatonCell().setDoNotUseFlag();
      }

      /// If one of the contained wire hits is marked as do not use this facet is set be not usable as well
      void receiveDoNotUse() const {

        if (getStartWireHit().getAutomatonCell().hasDoNotUseFlag() or
            getMiddleWireHit().getAutomatonCell().hasDoNotUseFlag() or
            getEndWireHit().getAutomatonCell().hasDoNotUseFlag()) {

          getAutomatonCell().setDoNotUseFlag();
        }
      }

      /// Getter for the automaton cell.
      AutomatonCell& getAutomatonCell() { return m_automatonCell; }

      /// Constant getter for the automaton cell.
      const AutomatonCell& getAutomatonCell() const { return m_automatonCell; }

    private:

      mutable ParameterLine2D m_startToMiddle; ///< Memory for the tangential line between first and second hit
      mutable ParameterLine2D m_startToEnd; ///< Memory for the tangential line between first and third hit
      mutable ParameterLine2D m_middleToEnd; ///< Memory for the tangential line between second and third hit

      AutomatonCell m_automatonCell; ///< Memory for the cellular automaton cell assoziated with the facet.



      /// ROOT Macro to make CDCRecoFacet a ROOT class.
      ClassDefInCDCLocalTracking(CDCRecoFacet, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCRECOFACET_H
