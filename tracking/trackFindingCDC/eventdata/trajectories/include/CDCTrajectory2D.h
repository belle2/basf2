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

#include <cmath>
#include <TMath.h>

#include <tracking/trackFindingCDC/numerics/BasicTypes.h>
#include <tracking/trackFindingCDC/numerics/InfoTypes.h>

#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/geometry/BoundSkewLine.h>

#include <tracking/trackFindingCDC/geometry/UncertainPerigeeCircle.h>

#include <tracking/trackFindingCDC/topology/ISuperLayerType.h>

#include <TMath.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /// Particle trajectory as it is seen in xy projection
    class CDCTrajectory2D  {

    public:
      /// Default constructor for ROOT compatibility.
      CDCTrajectory2D() :
        m_localOrigin(),
        m_localPerigeeCircle() {;}

      /// Constructs a trajectory from a generalized circle.
      /** Constructs a trajectory which is described by the given circle \n
       *  The start point is set to the closest approach to the origin */
      explicit CDCTrajectory2D(const UncertainPerigeeCircle& perigeeCircle) :
        m_localOrigin(0.0, 0.0),
        m_localPerigeeCircle(perigeeCircle)
      {;}

      /// Constructs a trajectory from a generalized circle and a start point
      /** Constructs a trajectory which is described by the given circle and \n
       *  starts in the given point. To point is taken to the closest appoach \n
       *  on the circle. */
      CDCTrajectory2D(
        const Vector2D& localOrigin,
        const UncertainPerigeeCircle& localPerigeeCircle
      ) :
        m_localOrigin(localOrigin),
        m_localPerigeeCircle(localPerigeeCircle)
      {;}

      /// Construct a trajectory with given start point, momentum at the start point and given charge.
      CDCTrajectory2D(const Vector2D& startPoint,
                      const Vector2D& startMomentum,
                      const FloatType& charge,
                      const FloatType& bZ);

      /// Construct a trajectory with given start point, momentum at the start point and given charge.
      CDCTrajectory2D(const Vector2D& startPoint,
                      const Vector2D& startMomentum,
                      const FloatType& charge);

    public:
      /// Checks if the circle is already set to a valid value
      bool isFitted() const
      { return not getLocalCircle().isNull(); }

      /// Calculate the travel distance from the start position of the trajectory.
      /** Returns the travel distance on the trajectory from the start point to \n
       *  the given point. This is subjected to a discontinuity at the far point \n
       *  of the circle. Hence the value return is in the range from -PI*radius to PI*radius \n
       *  If you have a heavily curling track you have care about the feasibility of this \n
       *  calculation. */
      FloatType calcArcLength2D(const Vector2D& point) const
      { return getLocalCircle().arcLengthBetween(Vector2D(0.0, 0.0), point - getLocalOrigin()); }


      /// Calculate the travel distance between the two given positions
      /** Returns the travel distance on the trajectory from the first given point to \n
       *  the second given point. This is subjected to a discontinuity at the far point \n
       *  of the circle. Hence the value return is in the range from -PI*radius to PI*radius \n
       *  If you have a heavily curling track you have care about the feasibility of this \n
       *  calculation. */
      FloatType calcArcLength2DBetween(const Vector2D& fromPoint, const Vector2D& toPoint) const
      {
        return getLocalCircle().arcLengthBetween(fromPoint - getLocalOrigin(),
                                                 toPoint - getLocalOrigin());
      }

      /// Gives the three dimensional point which is on the skew line and has the given proper (signed) distance to the circle in the xy projection
      /** This method makes the reconstruction of the z coordinate possible by using the skewness \n
       *  stereo layer of the stereo wires.  The point is determined such that it is on the given \n
       *  skew line and has the (signed) distance given as optional second parameter to the circular trajectory in the xy projection. */
      Vector3D reconstruct3D(const BoundSkewLine& skewLine,
                             const FloatType& distance = 0.0) const;

    public:
      /// Calculates the closest approach on the trajectory to the global origin
      Vector2D getGlobalPerigee() const
      { return getGlobalCircle().perigee(); }

      /// Calculates the closest approach on the trajectory to the given point
      Vector2D getClosest(const Vector2D& point) const
      { return getLocalCircle().closest(point - getLocalOrigin()) + getLocalOrigin(); }

      /// Calculates the close point with the same cylindricalR on the trajectory to the given point
      /** This returns the point where the trajectory reaches as certain distance from the origin \n
       *  ( in the xy projection ). It is useful to estimate where the trajectory reaches a  \n
       *  specific wire layer. */
      Vector2D getCloseSameCylindricalR(const Vector2D& point) const
      { return getGlobalCircle().sameCylindricalR(point); }

      /// Calculates the point where the trajectory meets the outer wall of the CDC.
      /** This method returns the first point in forward flight direction from the start
       *  point of the trajectory where it meets the outer radius of the outer most layer.
       *  If the trajectory does not meet the CDC by the outer wall this will return Vector2D(nan,nan)
       */
      Vector2D getOuterExit() const;


      /// Calculates the point where the trajectory meets the inner wall of the CDC.
      /** This method returns the first point in forward flight direction from the start
       *  point of the trajectory where it meets the inner radius of the inner most layer.
       *  If the trajectory does not meet the CDC by the inner wall this will return Vector2D(nan,nan)
       */
      Vector2D getInnerExit() const;

      /// Calculates the point where the trajectory leaves the CDC.
      /** This method returns the first point in forward flight direction from the start
       *  point of the trajectory where it meets either the radius of the inner most layer
       *  or the outer radius of the outer most wall.
       *  If the trajectory does not leave the CDC by the inner or outer wall this will return Vector2D(nan,nan).
       */
      Vector2D getExit() const;

      /// Calculates the distance from point the trajectory as seen from the xy projection
      FloatType getDist2D(const Vector2D& point) const
      {  return getLocalCircle().distance(point - getLocalOrigin()); }

      /// Checks if the given point is to the right or to the left of the trajectory
      SignType isRightOrLeft(const Vector2D& point) const
      { return getLocalCircle().isRightOrLeft(point - getLocalOrigin()); }

      /// Getter for the maximal distance from the origin
      FloatType getMaximalCylindricalR() const
      { return  getGlobalCircle().maximalCylindricalR(); }

      /// Getter for the minimal distance from the origin - same as absolute value of the impact parameter
      FloatType getMinimalCylindricalR() const
      { return  getGlobalCircle().minimalCylindricalR(); }

      /// Getter for the signed impact parameter of the trajectory
      FloatType getGlobalImpact() const
      { return  getGlobalCircle().impact(); }

      /// Indicates if the trajectory is moving outwards or inwards (to or away from the origin) from the start point on
      bool isMovingOutward() const
      { return getStartUnitMom2D().dot(getLocalOrigin()) > 0; }

    private:
      /// Indicates which superlayer is traversed after the given one, considering if you want to follow the trajectory in the forward or backward direction and if the trajectory is currently moving outward or inward (interpreted in the forward direction) or might curling back in the current layer.
      ISuperLayerType getISuperLayerAfter(const ISuperLayerType& fromISuperLayer,
                                          bool movingOutward,
                                          const ForwardBackwardInfo& forwardBackwardInfo) const;
      /// Indicates which superlayer is traversed after the one, where the start point of the trajectory is located considering, if you want to follow the trajectory in the forward or backward direction.
      ISuperLayerType getISuperLayerAfterStart(const ForwardBackwardInfo& forwardBackwardInfo) const;

      /// Indicates which axial superlayer is traversed after the one, where the start point of the trajectory is located considering, if you want to follow the trajectory in the forward or backward direction.
      ISuperLayerType getAxialISuperLayerAfterStart(const ForwardBackwardInfo& forwardBackwardInfo) const;

    public:
      /// Indicates which superlayer the trajectory traverses after the one, where the start point of the trajectory is located.
      ISuperLayerType getNextISuperLayer() const;

      /// Indicates which superlayer the trajectory traverses before the one, where the start point of the trajectory is located.
      ISuperLayerType getPreviousISuperLayer() const;

      /// Indicates which axial superlayer the trajectory traverses after the one, where the start point of the trajectory is located.
      ISuperLayerType getNextAxialISuperLayer() const;

      /// Indicates which axial superlayer the trajectory traverses before the one, where the start point of the trajectory is located.
      ISuperLayerType getPreviousAxialISuperLayer() const;



      /// Indicates the maximal superlayer the trajectory traverses
      ISuperLayerType getMaximalISuperLayer() const;

      /// Indicates the superlayer the trajectory starts in.
      ISuperLayerType getStartISuperLayer() const;

      /// Indicates the minimal superlayer the trajectory traverses
      ISuperLayerType getMinimalISuperLayer() const;

    public:
      /// Calculates the perpendicular travel distance from the last position of the fromHits to the first position of the toHits.
      template<class FromHits, class ToHits>
      FloatType getArcLength2DGap(const FromHits& fromHits, const ToHits& toHits) const
      {
        const Vector2D& fromRecoPos2D = fromHits.back().getRecoPos2D();
        const Vector2D& toRecoPos2D = toHits.front().getRecoPos2D();
        return calcArcLength2DBetween(fromRecoPos2D, toRecoPos2D);
      }


      /// Calculates the perpendicular travel distance from the first position of the fromHits to the first position of the toHits.
      template<class FromHits, class ToHits>
      FloatType getArcLength2DFrontOffset(const FromHits& fromHits, const ToHits& toHits) const
      {
        const Vector2D& fromRecoPos2D = fromHits.front().getRecoPos2D();
        const Vector2D& toRecoPos2D = toHits.front().getRecoPos2D();
        return calcArcLength2DBetween(fromRecoPos2D, toRecoPos2D);
      }

      /// Calculates the perpendicular travel distance from the last position of the fromHits to the last position of the toHits.
      template<class FromHits, class ToHits>
      FloatType getArcLength2DBackOffset(const FromHits& fromHits, const ToHits& toHits) const
      {
        const Vector2D& fromRecoPos2D = fromHits.back().getRecoPos2D();
        const Vector2D& toRecoPos2D = toHits.back().getRecoPos2D();
        return calcArcLength2DBetween(fromRecoPos2D, toRecoPos2D);
      }

      /// Calculates the perpendicular travel distance from the first position of the hits to the last position of the hits.
      template<class Hits>
      FloatType getTotalArcLength2D(const Hits& hits) const
      {
        Vector2D frontRecoPos2D = hits.front().getRecoPos2D();
        Vector2D backRecoPos2D = hits.back().getRecoPos2D();
        return calcArcLength2DBetween(frontRecoPos2D, backRecoPos2D);
      }

      /** Calculates if this trajectory and the hits are coaligned
       *  Returns:
       *  * FORWARD if the last entity lies behind the first.
       *  * BACKWARD if the last entity lies before the first.
       */
      template<class Hits>
      ForwardBackwardInfo isForwardOrBackwardTo(const Hits& hits) const
      { return sign(getTotalArcLength2D(hits)); }

    public:
      /// Gets the charge sign of the trajectory
      SignType getChargeSign() const;

      /// Reverses the trajectory in place
      void reverse()
      { m_localPerigeeCircle.reverse(); }

      /// Returns the reverse trajectory as a copy
      CDCTrajectory2D reversed() const
      { return CDCTrajectory2D(getLocalOrigin(), getLocalCircle().reversed()) ; }

      /// Get unit momentum vector at a specific postion
      /** Return the unit travel direction at the closest approach to the position */
      inline Vector2D getUnitMom2D(const Vector2D& point) const
      { return getLocalCircle().tangential(point - getLocalOrigin()); }

      /// Get the estimation for the absolute value of the transvers momentum
      FloatType getAbsMom2D(const FloatType& bZ) const;

      /// Get the estimation for the absolute value of the transvers momentum
      FloatType getAbsMom2D() const;

      /// Get the momentum at the support point of the trajectory
      inline Vector2D getMom2DAtSupport(const FloatType& bZ) const
      { return  getStartUnitMom2D() *= getAbsMom2D(bZ);  }

      /// Get the momentum at the support point of the trajectory
      inline Vector2D getMom2DAtSupport() const
      { return  getStartUnitMom2D() *= getAbsMom2D();  }

      /// Get the momentum at the start point of the trajectory
      inline Vector2D getStartMom2D() const
      { return  getStartUnitMom2D() *= getAbsMom2D();  }

      /// Get the unit momentum at the start point of the trajectory
      inline Vector2D getStartUnitMom2D() const
      { return  getLocalCircle().tangential();  }

      /// Setter for start point and momentum at the start point subjected to the charge sign.
      void setPosMom2D(const Vector2D& pos2D, const Vector2D& mom2D, const FloatType& charge);

      /// Get the support point of the trajectory in global coordinates
      inline Vector2D getSupport() const
      { return getLocalCircle().perigee() + getLocalOrigin(); }


      /// Clears all information from this trajectoy
      void clear()
      {
        m_localOrigin.set(0.0, 0.0);
        m_localPerigeeCircle.setNull();
      }



    public:
      /// Getter for the curvature as seen from the xy projection.
      FloatType getCurvature() const
      { return getLocalCircle().curvature(); }

      /// Getter for an individual element of the covariance matrix of the local helix parameters.
      FloatType getLocalCovariance(PerigeeParameterIndex iRow, PerigeeParameterIndex iCol) const
      { return getLocalCircle().covariance(iRow, iCol); }

      /// Getter for an individual diagonal element of the covariance matrix of the local helix parameters.
      FloatType getLocalVariance(PerigeeParameterIndex i) const
      { return getLocalCircle().variance(i); }


      /// Getter for the circle in global coordinates
      GeneralizedCircle getGlobalCircle() const
      {
        // Down cast since we do not necessarily wont the covariance matrix transformed as well
        GeneralizedCircle result(m_localPerigeeCircle);
        result.passiveMoveBy(-getLocalOrigin());
        return result;
      }

      /// Setter for the generalized circle that describes the trajectory.
      void setGlobalCircle(const UncertainPerigeeCircle& perigeeCircle)
      {
        m_localPerigeeCircle = perigeeCircle;
        m_localPerigeeCircle.passiveMoveBy(getLocalOrigin());
      }


      /// Getter for the cirlce in local coordinates
      const UncertainPerigeeCircle& getLocalCircle() const
      { return m_localPerigeeCircle; }

      ///  Getter for p-value
      FloatType getPValue() const
      { return TMath::Prob(getChi2(), getNDF()); }

      /// Getter for the chi2 value of the circle fit
      FloatType getChi2() const
      { return getLocalCircle().chi2(); }

      /// Setter for the chi square value of the circle fit
      void setChi2(const FloatType& chi2)
      { return m_localPerigeeCircle.setChi2(chi2); }

      /// Getter for the number of degrees of freedom of the circle fit.
      size_t getNDF() const
      { return getLocalCircle().ndf(); }

      /// Setter for the number of degrees of freedom of the circle fit.
      void setNDF(const size_t& ndf)
      { return m_localPerigeeCircle.setNDF(ndf); }

      /// Setter for the generalized circle that describes the trajectory.
      void setLocalCircle(const UncertainPerigeeCircle& localPerigeeCircle)
      { m_localPerigeeCircle = localPerigeeCircle; }



      /// Getter for the origin of the local coordinate system
      const Vector2D& getLocalOrigin() const
      { return m_localOrigin; }

      /// Setter for the origin of the local coordinate system.
      /** This sets the origin point the local circle representation is subjected. The local circle is also changed such that the set of points in global space is not changed on repositioning the local parameterisation.
       *  It also implicitly sets up a new reference position for all travel distances. \n
       *  To be able to keep track how the travel distances have to be shifted by this change \n
       *  the setter returns the value by which the coordinate s parameter was moved ( passively ). \n
       *  The relation "old traveldistance - return value == new traveldistance" holds. \n
       *  (if they are not to far away from the reference points, up to the discontinuity at the \n
       *  far point on the circle) \n
       *  @return Travel distance from the old to the new origin point */
      FloatType setLocalOrigin(const Vector2D& localOrigin)
      {
        FloatType result = calcArcLength2D(localOrigin);
        m_localPerigeeCircle.passiveMoveBy(localOrigin - m_localOrigin);
        m_localOrigin = localOrigin;
        return result;
      }

    public:
      /// Output helper for debugging
      friend std::ostream& operator<<(std::ostream& output, const CDCTrajectory2D& trajectory2D)
      {
        return output << "Local origin : " << trajectory2D.getLocalOrigin() <<  ", "
               << "local circle : " << trajectory2D.getLocalCircle();
      }

    private:
      /// Memory for local coordinate origin of the circle representing the trajectory in global coordinates
      Vector2D m_localOrigin;

      /// Memory for the generalized circle describing the trajectory in coordinates from the local origin
      UncertainPerigeeCircle m_localPerigeeCircle;
    }; //class

  } // namespace TrackFindingCDC
} // namespace Belle2
