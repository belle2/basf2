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

#include <tracking/trackFindingCDC/topology/ISuperLayer.h>

#include <tracking/trackFindingCDC/geometry/UncertainPerigeeCircle.h>
#include <tracking/trackFindingCDC/geometry/PerigeeCircle.h>
#include <tracking/trackFindingCDC/geometry/PerigeeParameters.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <tracking/trackFindingCDC/numerics/EForwardBackward.h>
#include <tracking/trackFindingCDC/numerics/ERightLeft.h>
#include <tracking/trackFindingCDC/numerics/ESign.h>

#include <TMath.h>
#include <cmath>
#include <cstddef>
#include <iosfwd>

namespace Belle2 {
  namespace TrackFindingCDC {
    class WireLine;

    /// Particle trajectory as it is seen in xy projection represented as a circle.
    class CDCTrajectory2D  {

    public:
      /// Default constructor for ROOT compatibility.
      CDCTrajectory2D();

      /**
       *  Constructs a trajectory from a generalized circle.
       *  Constructs a trajectory which is described by the given line or circle.
       *  The start point is set to the closest approach to the origin.
       */
      explicit CDCTrajectory2D(const UncertainPerigeeCircle& perigeeCircle);

      /**
       *  Constructs a trajectory from a generalized circle and a start point.
       *
       *  Constructs a trajectory which is described by the given line or circle and
       *  starts in the given point. The point is taken to be the closest approach
       *  to the circle.
       */
      CDCTrajectory2D(const Vector2D& localOrigin,
                      const UncertainPerigeeCircle& localPerigeeCircle,
                      double flightTime = NAN);

      /**
       *  Construct a trajectory with given start point, transverse momentum at the start point,
       *  the given charge and the magnetic field value in z direction.
       */
      CDCTrajectory2D(const Vector2D& pos2D,
                      double time,
                      const Vector2D& mom2D,
                      double charge,
                      double bZ);

      /// Construct a trajectory with given start point, transverse momentum at the start point and
      /// given charge.
      CDCTrajectory2D(const Vector2D& pos2D, double time, const Vector2D& mom2D, double charge);

    public:
      /// Checks if the circle is already set to a valid value.
      bool isFitted() const;

      /// Clears all information from this trajectoy
      void clear();

    public:
      /// Reverses the trajectory in place
      void reverse();

      /// Returns the reverse trajectory as a copy
      CDCTrajectory2D reversed() const;

    public:
      /**
       *  Gives the two z postions where the given drift circle on the wire line touches the trajectory
       *
       *  Only works for the skew stereo wires
       *
       *  @param wireLine  The geometrical wire line on which the hit is located-
       *  @param distance  The desired distance from the wire line a.k.a. drift length
       *  @param z         The expected value of z to which to closest solution should be selected.
       */
      std::array<double, 2> reconstructBothZ(const WireLine& wireLine, double distance = 0.0, double z = 0) const;

      /**
       *  Gives the one z postions within the CDC closest to the given z
       *  where the given drift circle on the wire line touches the trajectory.
       *
       *  Only works for the skew stereo wires.
       *
       *  @param wireLine  The geometrical wire line on which the hit is located-
       *  @param distance  The desired distance from the wire line a.k.a. drift length
       *  @param z         The expected value of z to which to closest solution should be selected.
       */
      double reconstructZ(const WireLine& wireLine, double distance = 0.0, double z = 0) const;

      /**
       *  Gives the two three dimensional points where the drift circle touches the wire line.
       *
       *  Only works for the skew stereo wires.
       *
       *  @param wireLine  The geometrical wire line on which the hit is located-
       *  @param distance  The desired distance from the wire line a.k.a. drift length
       *  @param z         The expected value of z to which to closest solution should be selected.
       */
      std::array<Vector3D, 2> reconstructBoth3D(const WireLine& wireLine, double distance = 0.0, double z = 0) const;

      /**
       *  Gives the one three dimensional postions within the CDC  closest to the given z
       *  where the given drift circle on the wire line touches the trajectory.
       *
       *  This method makes the reconstruction of the z coordinate possible by using the skewness \n
       *  stereo layer of the stereo wires.  The point is determined such that it is at the (signed)
       *  distance to  the wire line.
       *
       *  @param wireLine  The geometrical wire line on which the hit is located-
       *  @param distance  The desired distance from the wire line a.k.a. drift length
       *  @param z         The expected value of z to which to closest solution should be selected.
       */
      Vector3D reconstruct3D(const WireLine& wireLine, double distance = 0.0, double z = 0) const;

      /// Calculates the closest approach on the trajectory to the given point
      Vector2D getClosest(const Vector2D& point) const;

    private:
      /**
       * Returns which superlayer is traversed after the current one following
       * the trajectory outward or inward as indicated by the boolean input.
       */
      ISuperLayer getISuperLayerAfter(ISuperLayer iSuperLayer, bool movingOutward) const;

      /**
       *  Returns which superlayer is traversed after the current one following
       *  the trajectory outward or inward as indicated by the boolean input.
       */
      ISuperLayer getISuperLayerAfterStart(bool movingOutward) const;

      /**
       *  Indicates which superlayer is traversed after the current one following
       *  the trajectory forward or backward as indicated by the input.
       */
      ISuperLayer getISuperLayerAfterStart(EForwardBackward forwardBackwardInfo) const;

      /**
       *  Indicates which axial superlayer is traversed after the one, where the start point of the
       * trajectory is located considering
       *  if you want to follow the trajectory in the forward or backward direction.
       */
      ISuperLayer getAxialISuperLayerAfterStart(EForwardBackward forwardBackwardInfo) const;

    public:
      /// Indicates which superlayer the trajectory traverses after the one, where the start point
      /// of the trajectory is located.
      ISuperLayer getNextISuperLayer() const;

      /// Indicates which superlayer the trajectory traverses before the one, where the start point
      /// of the trajectory is located.
      ISuperLayer getPreviousISuperLayer() const;

      /// Indicates which axial superlayer the trajectory traverses after the one, where the start
      /// point of the trajectory is located.
      ISuperLayer getNextAxialISuperLayer() const;

      /// Indicates which axial superlayer the trajectory traverses before the one, where the start
      /// point of the trajectory is located.
      ISuperLayer getPreviousAxialISuperLayer() const;

      /// Indicates the maximal superlayer the trajectory traverses
      ISuperLayer getMaximalISuperLayer() const;

      /// Indicates the superlayer the trajectory starts in.
      ISuperLayer getStartISuperLayer() const;

      /// Indicates the minimal superlayer the trajectory traverses
      ISuperLayer getMinimalISuperLayer() const;

    public:
      /**
       *  Calculates if this trajectory and the hits are coaligned
       *  Returns:
       *  * EForwardBackward::c_Forward if the last entity lies behind the first.
       *  * EForwardBackward::c_Backward if the last entity lies before the first.
       */
      template <class AHits>
      EForwardBackward isForwardOrBackwardTo(const AHits& hits) const
      {
        return static_cast<EForwardBackward>(sign(getTotalArcLength2D(hits)));
      }

      /**
       *  Calculates the perpendicular travel distance from the last position of the fromHits
       *  to the first position of the toHits.
       */
      template <class AFromHits, class AToHits>
      double getArcLength2DGap(const AFromHits& fromHits, const AToHits& toHits) const
      {
        const Vector2D& fromRecoPos2D = fromHits.back().getRecoPos2D();
        const Vector2D& toRecoPos2D = toHits.front().getRecoPos2D();
        return calcArcLength2DBetween(fromRecoPos2D, toRecoPos2D);
      }

      /**
       *  Calculates the perpendicular travel distance from the first position
       *  of the fromHits to the first position of the toHits.
       */
      template <class AFromHits, class AToHits>
      double getArcLength2DFrontOffset(const AFromHits& fromHits, const AToHits& toHits) const
      {
        const Vector2D& fromRecoPos2D = fromHits.front().getRecoPos2D();
        const Vector2D& toRecoPos2D = toHits.front().getRecoPos2D();
        return calcArcLength2DBetween(fromRecoPos2D, toRecoPos2D);
      }

      /**
       *  Calculates the perpendicular travel distance from the last position
       *  of the fromHits to the last position of the toHits.
       */
      template <class AFromHits, class AToHits>
      double getArcLength2DBackOffset(const AFromHits& fromHits, const AToHits& toHits) const
      {
        const Vector2D& fromRecoPos2D = fromHits.back().getRecoPos2D();
        const Vector2D& toRecoPos2D = toHits.back().getRecoPos2D();
        return calcArcLength2DBetween(fromRecoPos2D, toRecoPos2D);
      }

      /// Calculates the perpendicular travel distance from the first position of the hits to the
      /// last position of the hits.
      template <class AHits>
      double getTotalArcLength2D(const AHits& hits) const
      {
        Vector2D frontRecoPos2D = hits.front().getRecoPos2D();
        Vector2D backRecoPos2D = hits.back().getRecoPos2D();
        return calcArcLength2DBetween(frontRecoPos2D, backRecoPos2D);
      }

      /**
       *  Calculate the travel distance from the start position of the trajectory.
       *
       *  Returns the travel distance on the trajectory from the start point to \n
       *  the given point. This is subjected to a discontinuity at the far point \n
       *  of the circle. Hence the value return is in the range from -pi*radius to pi*radius \n
       *  If you have a heavily curling track you have care about the feasibility of this \n
       *  calculation.
       */
      double calcArcLength2D(const Vector2D& point) const
      {
        return getLocalCircle()->arcLengthBetween(Vector2D(0.0, 0.0), point - getLocalOrigin());
      }

      /**
       *  Calculate the travel distance between the two given positions
       *  Returns the travel distance on the trajectory from the first given point to \n
       *  the second given point. This is subjected to a discontinuity at the far point \n
       *  of the circle. Hence the value return is in the range from -pi*radius to pi*radius \n
       *  If you have a heavily curling track you have care about the feasibility of this \n
       *  calculation.
       */
      double calcArcLength2DBetween(const Vector2D& fromPoint, const Vector2D& toPoint) const
      {
        return getLocalCircle()->arcLengthBetween(fromPoint - getLocalOrigin(),
                                                  toPoint - getLocalOrigin());
      }
      /// Getter for the arc length for one round trip around the trajectory.
      double getArcLength2DPeriod() const
      {
        return getLocalCircle()->arcLengthPeriod();
      }

    public:
      /// Setter for start point and momentum at the start point subjected to the charge sign.
      void setPosMom2D(const Vector2D& pos2D, const Vector2D& mom2D, double charge);

      /// Gets the charge sign of the trajectory
      ESign getChargeSign() const;

      /// Get the estimation for the absolute value of the transvers momentum
      double getAbsMom2D(double bZ) const;

      /// Get the estimation for the absolute value of the transvers momentum
      double getAbsMom2D() const;

      /// Get the momentum at the support point of the trajectory
      Vector2D getMom2DAtSupport(const double bZ) const
      {
        return getFlightDirection2DAtSupport() *= getAbsMom2D(bZ);
      }

      /// Get the momentum at the support point of the trajectory
      Vector2D getMom2DAtSupport() const
      {
        return getFlightDirection2DAtSupport() *= getAbsMom2D();
      }

      /// Get the unit direction of flight at the given point, where arcLength2D = 0.
      Vector2D getFlightDirection2D(const Vector2D& point) const
      {
        return getLocalCircle()->tangential(point - getLocalOrigin());
      }

      /// Get the unit direction of flight at the support point, where arcLength2D = 0.
      Vector2D getFlightDirection2DAtSupport() const
      {
        return getLocalCircle()->tangential();
      }

      /// Indicates if the trajectory is moving outwards or inwards (to or away from the origin)
      /// from the start point on
      bool isMovingOutward() const
      {
        return getFlightDirection2DAtSupport().dot(getSupport()) > 0;
      }

      /// Getter for the position at a given two dimensional arc length
      Vector2D getPos2DAtArcLength2D(double arcLength2D)
      {
        return getLocalOrigin() + getLocalCircle()->atArcLength(arcLength2D);
      }

      /// Get the support point of the trajectory in global coordinates
      Vector2D getSupport() const
      {
        return getLocalCircle()->perigee() + getLocalOrigin();
      }

      /// Getter for the closest approach on the trajectory to the global origin
      Vector2D getGlobalPerigee() const
      {
        return getLocalCircle()->closest(-m_localOrigin) + m_localOrigin;
      }

      /// Getter for the center of the trajectory in global coordinates
      Vector2D getGlobalCenter() const
      {
        return getLocalCircle()->center() + m_localOrigin;
      }

      /**
       *  Calculates the point where the trajectory meets the outer wall of the CDC.
       *  This method returns the first point in forward flight direction from the start
       *  point of the trajectory where it meets the outer radius of the outer most layer.
       *  If the trajectory does not meet the CDC by the outer wall this will return
       * Vector2D(nan,nan)
       *  The factor can be used to virtually resize the CDC.
       */
      Vector2D getOuterExit(double factor = 1) const;

      /**
       *  Calculates the point where the trajectory meets the inner wall of the CDC.
       *  This method returns the first point in forward flight direction from the start
       *  point of the trajectory where it meets the inner radius of the inner most layer.
       *  If the trajectory does not meet the CDC by the inner wall this will return
       * Vector2D(nan,nan)
       */
      Vector2D getInnerExit() const;

      /**
       *  Calculates the point where the trajectory leaves the CDC.
       *  This method returns the first point in forward flight direction from the start
       *  point of the trajectory where it meets either the radius of the inner most layer
       *  or the outer radius of the outer most wall.
       *  If the trajectory does not leave the CDC by the inner or outer wall this will return
       * Vector2D(nan,nan).
       */
      Vector2D getExit() const;

      /// Checks if the trajectory leaves the outer radius of the CDC times the given tolerance
      /// factor
      bool isCurler(double factor = 1) const;

      /// Checks if the trajectory intersects with the inner radius of the CDC time the given
      /// tolerance factor
      bool isOriginer(double factor = 1) const;

      /// Getter for the maximal distance from the origin
      double getMaximalCylindricalR() const
      {
        return std::fabs(getGlobalImpact() + 2 * getLocalCircle()->radius());
      }

      /// Getter for the minimal distance from the origin - same as absolute value of the impact
      /// parameter
      double getMinimalCylindricalR() const
      {
        return std::fabs(getGlobalImpact());
      }

      /// Getter for the signed impact parameter of the trajectory
      double getGlobalImpact() const
      {
        return getLocalCircle()->distance(-m_localOrigin);
      }

      /// Calculates the distance from the point to the trajectory as seen from the xy projection.
      double getDist2D(const Vector2D& point) const
      {
        return getLocalCircle()->distance(point - getLocalOrigin());
      }

      /// Checks if the given point is to the right or to the left of the trajectory
      ERightLeft isRightOrLeft(const Vector2D& point) const
      {
        return getLocalCircle()->isRightOrLeft(point - getLocalOrigin());
      }

    public:
      /// Getter for the curvature as seen from the xy projection.
      double getCurvature() const
      {
        return getLocalCircle()->curvature();
      }

      /// Getter for an individual element of the covariance matrix of the local helix parameters.
      double getLocalCovariance(EPerigeeParameter iRow, EPerigeeParameter iCol) const
      {
        return getLocalCircle().covariance(iRow, iCol);
      }

      /// Getter for an individual diagonal element of the covariance matrix of the local helix
      /// parameters.
      double getLocalVariance(EPerigeeParameter i) const
      {
        return getLocalCircle().variance(i);
      }

      /// Getter for the circle in global coordinates
      PerigeeCircle getGlobalCircle() const
      {
        PerigeeCircle result = getLocalCircle();
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
      {
        return m_localPerigeeCircle;
      }

      ///  Getter for p-value
      double getPValue() const
      {
        return TMath::Prob(getChi2(), getNDF());
      }

      /// Getter for the chi2 value of the circle fit
      double getChi2() const
      {
        return getLocalCircle().chi2();
      }

      /// Setter for the chi square value of the circle fit
      void setChi2(const double chi2)
      {
        return m_localPerigeeCircle.setChi2(chi2);
      }

      /// Getter for the number of degrees of freedom of the circle fit.
      size_t getNDF() const
      {
        return getLocalCircle().ndf();
      }

      /// Setter for the number of degrees of freedom of the circle fit.
      void setNDF(std::size_t ndf)
      {
        return m_localPerigeeCircle.setNDF(ndf);
      }

      /// Setter for the generalized circle that describes the trajectory.
      void setLocalCircle(const UncertainPerigeeCircle& localPerigeeCircle)
      {
        m_localPerigeeCircle = localPerigeeCircle;
      }

      /// Getter for the origin of the local coordinate system
      const Vector2D& getLocalOrigin() const
      {
        return m_localOrigin;
      }

      /**
       *  Setter for the origin of the local coordinate system.
       *
       *  This sets the origin point the local circle representation is subjected.
       *  The local circle is also changed such that the set of points in global space
       *  is not changed on repositioning the local parameterisation.
       *  It also implicitly sets up a new reference position for all travel distances. \n
       *  To be able to keep track how the travel distances have to be shifted by this change \n
       *  the setter returns the value by which the coordinate s parameter was moved ( passively ). \n
       *  The relation "old traveldistance - return value == new traveldistance" holds. \n
       *  (if they are not to far away from the reference points, up to the discontinuity at the \n
       *  far point on the circle) \n
       *
       *  @param localOrigin  New local reference point in the global coordinate system
       *  @return             Travel distance from the old to the new origin point
       */
      double setLocalOrigin(const Vector2D& localOrigin);

      /// Getter for the time when the particle reached the support point position.
      double getFlightTime() const
      {
        return m_flightTime;
      }

      /// Setter for the time when the particle reached the support point position.
      void setFlightTime(double flightTime)
      {
        m_flightTime = flightTime;
      }

    private:
      /// Memory for local coordinate origin of the circle representing the trajectory in global coordinates
      Vector2D m_localOrigin;

      /// Memory for the generalized circle describing the trajectory in coordinates from the local origin
      UncertainPerigeeCircle m_localPerigeeCircle;

      /// Memory for the estimation of the time at which the particle arrived at the support point
      double m_flightTime = NAN;
    };

    /// Output helper for debugging
    std::ostream& operator<<(std::ostream& output, const CDCTrajectory2D& trajectory2D);
  }
}
