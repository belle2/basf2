/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCRECOHITTANGENT_H
#define CDCRECOHITTANGENT_H

#include <vector>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <tracking/cdcLocalTracking/geometry/ParameterLine2D.h>

#include "CDCRLWireHit.h"
#include "CDCRLWireHitPair.h"

#include "CDCRecoHit2D.h"

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class representating a linear track piece between two oriented wire hits \n
    /** A reconstructed tangent is an approximation of the possible trajectory between two oriented wire hits. \n
     *  The approximation is obtained by constructing a tangent between two drift circles of the wire hits  \n
     *  ( in the reference xy projection ) \n
     *  Generally maximal four tangents are possible. So to uniquely define a tangent we have \n
     *  to give additional information how it passes relativ to the drift circles. This right left \n
     *  passage inforamtion indicates if the related wire hit should lie to the right of to the left \n
     *  of the tangent. The four possible combinations are then ( RIGHT , RIGHT ), ( RIGHT , LEFT ), \n
     *  ( LEFT , RIGHT ), ( LEFT , LEFT ). \n
     *  To represent the tangent this class uses a ParameterLine2D. The touch points to the two drift circles \n
     *  are located at(0) for the first and at(1) for the second. \n
     *  The tangent has therefor a sense of what is forward and can be reversed if necessary \n
     *  Generally tangents are only a good approximation between neighboring wire hits.*/
    class CDCRecoTangent : public CDCRLWireHitPair {
    public:

      /// Constructs a line touching two circles in one point each.
      /** @param fromCenter first circle center
       *  @param signedFromRadius radius of the first circle multiplied with the right left passage information
       *  @param toCenter second circle center
       *  @param signedToRadius radius of the first circle multiplied with the right left passage information.
       *  @return the line being tangential to both circles.
       *  Note : the touch points reside at(0) for the first and at(1) for the second */
      static ParameterLine2D constructTouchingLine(
        const Vector2D& fromCenter,
        const FloatType& fromSignedRadius,
        const Vector2D& toCenter,
        const FloatType& toSignedRadius
      );


    public:
      /// Default constructor for ROOT compatibility.
      CDCRecoTangent();

      /// Construct a tangent from a pair of oriented wire hits
      CDCRecoTangent(const CDCRLWireHitPair& rlWireHitPair);

      /// Construct a tangent from two oriented wire hits.
      CDCRecoTangent(const CDCRLWireHit* fromRLWireHit, const CDCRLWireHit* toRLWireHit);

      /// Construct a tangent from a pair of oriented wire hits taking the given tangential line instead of a computed one.
      CDCRecoTangent(const CDCRLWireHitPair& rlWireHitPair, const ParameterLine2D& line);

      /// Construct a tangent from two oriented wire hits taking the given tangential line instead of a computed one.
      CDCRecoTangent(const CDCRLWireHit* fromRLWireHit, const CDCRLWireHit* toRLWireHit, const ParameterLine2D& line);

      /// Construct a tangent from two reconstructed hits
      CDCRecoTangent(const CDCRecoHit2D& fromRecoHit, const CDCRecoHit2D& toRecoHit);

      /// Empty deconstructor
      ~CDCRecoTangent();



      /// Print tangent for debugging
      friend ostream& operator<<(ostream& output, const CDCRecoTangent& tangent) {
        output << "RecoTangent" << std::endl;
        output << "From : " << tangent.getFromWireHit()->getWire() << " " <<  tangent.getFromRefDisp2D() << std::endl;
        output << "To : " << tangent.getToWireHit()->getWire() << " " <<  tangent.getToRefDisp2D()  << std::endl;
        return output;
      }

      /// Access the object methods and methods from a pointer in the same way.
      /** In situations where the type is not known to be a pointer or a reference there is no way to tell \n
       *  if one should use the dot '.' or operator '->' for method look up. \n
       *  So this function defines the -> operator for the object. \n
       *  No matter you have a pointer or an object access is given with '->'*/
      const CDCRecoTangent* operator->() const { return this; }



      /// Getter for the touching point of the tangent to the first drift circle
      const Vector2D& getFromRefTouch2D() const
      { return getLine().support(); }

      /// Getter for displacement of the touching point from the first wire in the reference plane
      Vector2D getFromRefDisp2D() const
      { return getFromRefTouch2D() - getFromWireHit()->getRefPos2D(); }

      /// Getter for the touching point of the tangent to the second drift circle
      Vector2D getToRefTouch2D() const
      { return  getLine().at(1); }

      /// Getter for displacement of the touching point from the second wire in the reference plane
      Vector2D getToRefDisp2D() const
      { return getToRefTouch2D() - getToWireHit()->getRefPos2D(); }

      /// Getter for the vector from the first to the second touch point.*/
      const Vector2D& getRefFlightVec2D() const
      { return getLine().tangential(); }

      /// Returns the cosine of the angle between the two flight directions of the tangents.
      double getCosFlightDifference(const CDCRecoTangent& tangent) const
      { return getRefFlightVec2D().cosWith(tangent.getRefFlightVec2D()); }

      /// Gets the center of the tangent half way from one touch point to the other
      Vector2D getCenterOfMass2D() const
      { return getLine().at(0.5) ; }



      /// Getter for the reconstructed hit on the first oriented wire hit using reconstructed touch point as position
      CDCRecoHit2D getFromRecoHit2D() const
      { return CDCRecoHit2D::fromAbsPos2D(&(getFromWireHit()), getFromRLInfo(), getFromRefTouch2D()); }

      /// Getter for the reconstructed hit on the second oriented wire hit using reconstructed touch point as position
      CDCRecoHit2D getToRecoHit2D() const
      { return CDCRecoHit2D::fromAbsPos2D(&(getToWireHit()), getToRLInfo(), getToRefTouch2D()); }



      /// Estimate the transvers travel distance on the given circle of the first touch point
      /** Uses the point of closest approach on the circle
       *  to the first touch point and
       *  calculates the arc length from the reference point on the circle.
       *  @return The arc length on the circle from the reference */
      FloatType getStartPerpS(const CDCTrajectory2D& trajectory2D) const
      { return trajectory2D.calcPerpS(getFromRefTouch2D()); }

      /// Estimate the transvers travel distance on the given circle of the second touch point
      /** Uses the point of closest approach on the circle
       *  to the second touch point and
       *  calculates the arc length from the reference point on the circle.
       *  @return The arc length on the circle from the reference */
      FloatType getEndPerpS(const CDCTrajectory2D& trajectory2D) const
      { return trajectory2D.calcPerpS(getToRefTouch2D()); }

      /// Calculates the squared distance of the tangent to a circle as see from the transvers plane.
      FloatType getSquaredDist2D(const CDCTrajectory2D& trajectory2D) const;

    public:
      /// Adjusts the line to touch the drift circles with the correct right left passage information
      void adjustLine();

      /// Adjusts the right left passage information according to the tangent line
      void adjustRLInfo();

      /// Reverses the tangent inplace.
      /** Swaps the to wire hits, reverses the left right informations and exchanges the two touch points of the line */
      void reverse();

      /// Same as reverse but returns a copy.
      CDCRecoTangent reversed() const;

    public:
      /// Get for the line representation of the line
      /** The line stretchs between the two the touch point. The first touch point is at(0) the second at(1). */
      const ParameterLine2D& getLine() const
      { return m_line; }

    private:
      ParameterLine2D m_line;///< Memory for the line between the two touching points. The first touch point at(0), second at(1).

      /// ROOT Macro to make CDCRecoTangent a ROOT class.
      ClassDefInCDCLocalTracking(CDCRecoTangent, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCRECOHITTANGENT_H
