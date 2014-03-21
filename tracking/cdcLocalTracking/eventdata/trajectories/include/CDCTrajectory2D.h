/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCTRAJECTORY2D_H
#define CDCTRAJECTORY2D_H

#include <cmath>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/typedefs/InfoTypes.h>

#include <tracking/cdcLocalTracking/geometry/Vector2D.h>
#include <tracking/cdcLocalTracking/geometry/Vector3D.h>
#include <tracking/cdcLocalTracking/geometry/BoundSkewLine.h>
#include <tracking/cdcLocalTracking/geometry/GeneralizedCircle.h>

namespace Belle2 {

  namespace CDCLocalTracking {

    /// Particle trajectory as it is seen in xy projection
    class CDCTrajectory2D : public CDCLocalTracking::UsedTObject {

    public:

      /// Default constructor for ROOT compatibility.
      CDCTrajectory2D() : m_genCircle(), m_startPos2D() {;}

      /// Constructs a trajectory from a generalized circle.
      /** Constructs a trajectory which is described by the given circle \n
       *  The start point is set to the closest approach to the origin */
      CDCTrajectory2D(const GeneralizedCircle& genCircle) :
        m_genCircle(genCircle),
        m_startPos2D(genCircle.closestToOrigin()) {;}

      /// Constructs a trajectory from a generalized circle and a start point
      /** Constructs a trajectory which is described by the given circle and \n
       *  starts in the given point. To point is taken to the closest appoach \n
       *  on the circle. */
      CDCTrajectory2D(
        const GeneralizedCircle& genCircle,
        const Vector2D& startPoint
      ) :
        m_genCircle(genCircle),
        m_startPos2D(genCircle.closest(startPoint)) {;}

      CDCTrajectory2D(const Vector2D& startPoint, const Vector2D& startMomentum, const FloatType& charge);

      /// Empty destructor
      ~CDCTrajectory2D() {;}

    public:

      /// Checks if the circle is already set to a valid value
      bool isFitted() const { return not getGenCircle().isNull(); }

      /// Calculate the travel distance from the start position of the trajectory.
      /** Returns the travel distance on the trajectory from the start point to \n
       *  the given point. This is subjected to a discontinuity at the far point \n
       *  of the circle. Hence the value return is in the range from -PI*radius to PI*radius \n
       *  If you have a heavily curling track you have care about the feasibility of this \n
       *  calculation. */
      FloatType calcPerpS(const Vector2D& point) const
      { return getGenCircle().lengthOnCurve(m_startPos2D, point); }


      /// Calculate the travel distance between the two given positions
      /** Returns the travel distance on the trajectory from the first given point to \n
       *  the second given point. This is subjected to a discontinuity at the far point \n
       *  of the circle. Hence the value return is in the range from -PI*radius to PI*radius \n
       *  If you have a heavily curling track you have care about the feasibility of this \n
       *  calculation. */
      FloatType calcPerpSBetween(const Vector2D& fromPoint, const Vector2D& toPoint) const
      { return getGenCircle().lengthOnCurve(fromPoint, toPoint); }

      /// Gives the three dimensional point which is on the skew line as well as on the circle in the xy projection
      /** This method makes the reconstruction of the z coordinate possble by using the skewness \n
       *  stereo layer of the stereo wires.  The point is determined such that it is on the given \n
       *  skew line as well as on the circular trajector in the xy projection. */
      Vector3D reconstruct3D(const BoundSkewLine& skewLine) const;

    public:

      /// Calculates the closest approach on the trajectory to the origin
      Vector2D getClosestToOrigin() const
      { return getGenCircle().closestToOrigin(); }

      /// Calculates the closest approach on the trajectory to the given point
      Vector2D getClosest(const Vector2D& point) const
      { return getGenCircle().closest(point); }

      /// Calculates the close point with the same polarR on the trajectory to the given point
      /** This returns the point where the trajectory reaches as certain distance from the origin \n
       *  ( in the xy projection ). It is useful to estimate where the trajectory reaches a  \n
       *  specific wire layer. */
      Vector2D getCloseSamePolarR(const Vector2D& point) const
      { return getGenCircle().samePolarR(point); }

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
      {  return getGenCircle().distance(point); }

      /// Checks if the given point is to the right or to the left of the trajectory
      SignType isRightOrLeft(const Vector2D& point) const
      { return getGenCircle().isRightOrLeft(point); }

      /// Getter for the maximal distance from the origin
      FloatType getMaximalPolarR() const
      { return  getGenCircle().maximalPolarR(); }

      /// Getter for the minimal distance from the origin - same as absolute value of the impact parameter
      FloatType getMinimalPolarR() const
      { return  getGenCircle().minimalPolarR(); }

      /// Getter for the signed impact parameter of the trajectory
      FloatType getImpact() const
      { return  getGenCircle().impact(); }

      /// Indicates if the trajectory is moving outwards or inwards (to or away from the origin) from the start point on
      bool isMovingOutward() const
      { return getStartUnitMom2D().dot(getStartPos2D()) > 0; }

    private:
      /// Indicates which superlayer is traversed after the given one, considering if you want to follow the trajectory in the forward or backward direction and if the trajectory is currently moving outward or inward (interpreted in the forward direction) or might curling back in the current layer.
      ISuperLayerType getISuperLayerAfter(const ISuperLayerType& fromISuperLayer, bool movingOutward,  const ForwardBackwardInfo& forwardBackwardInfo) const;
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

      /// Indicates the maximal superlayer the trajectory traverses
      ISuperLayerType getStartISuperLayer() const;

      /// Indicates the minimal superlayer the trajectory traverses
      ISuperLayerType getMinimalISuperLayer() const;

    private:
      // this class should not really be the provider of this information.
      // So use the corresponding functions which should eventually change to the correct
      // information provider.
      // definitions in source file

      /// Constant for the magnetic field strength in z direction ( in Tesla )
      static const FloatType c_bFieldZMagnitude; // = 1.5;

      /// Constant for the sign of the magnetic field in z direction. To be checked.
      static const SignType c_bFieldZSign;  // = PLUS; // to be checked

      /// Constant for the signed magnetic field strength in z direction ( in Tesla )
      static const FloatType c_bFieldZ; //  = c_bFieldZSign * c_bFieldZMagnitude;

    private:
      /// Getter for the absolute magnetic field strength in z direction ( in Tesla )
      inline const FloatType& getBFieldZMagnitude() const
      { return c_bFieldZMagnitude; }

      /// Getter for the sign of the magnetic field in z direction
      inline const SignType& getBFieldZSign() const
      { return c_bFieldZSign; }

      /// Getter for the signed of the magnetic field stength in z direction ( in Tesla )
      inline const FloatType& getBFieldZ() const
      { return c_bFieldZ; }

      /// Conversion helper for momenta to radii
      inline FloatType momToRadius(const FloatType& mom, const FloatType& charge) const
      { return mom / (std::fabs(charge) * getBFieldZMagnitude()  * 0.00299792458); }


      /// Conversion helper for radii to momenta
      inline FloatType radiusToMom(const FloatType& r) const
      { return getBFieldZMagnitude() * 0.00299792458 * r; }

      /// Conversion helper from clockwise or counterclockwise travel to the charge sign.
      /** Return the charge sign based on the travel direction on the fitted circle. \n
       *  With the Lorentz force F = q * v x B \n
       *  For positively charged particles we have \n
       *  Counterclockwise travel <-> Bz < 0 \n
       *  Clockwise travel        <-> Bz > 0 \n
       *  and opposite for negatively charged. \n
       *  Hence the charge sign is -CCWInfo * sign(Bz) */
      inline SignType ccwInfoToChargeSign(const CCWInfo& ccwInfo) const
      { return - ccwInfo * getBFieldZSign(); }

      /// Conversion helper from the charge sign to clockwise or counterclockwise travel
      inline SignType chargeSignToCCWInfo(const SignType& chargeSign) const
      { return - chargeSign * getBFieldZSign(); }

    public:

      /// Gets the charge sign of the trajectory
      SignType getChargeSign() const
      { return ccwInfoToChargeSign(getGenCircle().orientation()) ; }

      /// Reverses the trajectory in place
      void reverse()
      { m_genCircle.reverse(); }

      /// Returns the reverse trajectory as a copy
      CDCTrajectory2D reversed()
      { return CDCTrajectory2D(m_genCircle.reversed()) ; }

      /// Get unit momentum vector at a specific postion
      /** Return the unit travel direction at the closest approach to the position */
      inline Vector2D getUnitMom2D(const Vector2D& point) const
      { return getGenCircle().tangential(point); }

      /// Get the estimation for the absolute value of the transvers momentum
      inline FloatType getAbsMom2D() const
      { return radiusToMom(getGenCircle().radius()); }

      /// Get the momentum at the start point of the trajectory
      inline Vector2D getStartMom2D() const
      { return  getStartUnitMom2D() *= getAbsMom2D();  }

      /// Get the unit momentum at the start point of the trajectory
      inline Vector2D getStartUnitMom2D() const
      { return  getUnitMom2D(getStartPos2D());  }

      /// Get the start point of the track
      const Vector2D& getStartPos2D() const
      { return m_startPos2D; }

      /// Setter for the start point of the trajectory
      /** This takes the given point to the closest approach on the trajectory and \n
       *  sets it as new start position of the trajectory. \n
       *  Hence it also sets up a new reference position for all travel distances. \n
       *  To be able to keep track how the travel distances have to be shifted by this change \n
       *  the setter returns the value by which the coordinate s parameter was moved ( passively ). \n
       *  The relation "old traveldistance - return value == new traveldistance" holds. \n
       *  (if they are not to far away from the reference points, up to the discontinuity at the \n
       *  far point on the circle) \n
       *  @return Original travel distance to the new start point */
      FloatType setStartPos2D(const Vector2D& point);

      /// Setter for start point and momentum at the start point subjected to the charge sign.
      void setStartPosMom2D(const Vector2D& pos, const Vector2D& mom, FloatType charge);

      /// Clears all information from this trajectoy
      void clear() {
        m_genCircle.setNull();
        m_startPos2D.set(0.0, 0.0);
      }

    public:
      /// Getter for the generalized circle that describes the trajectory.
      const GeneralizedCircle& getGenCircle() const
      { return m_genCircle; }

      /// Setter for the generalized circle that describes the trajectory.
      void setGenCircle(const GeneralizedCircle& genCircle)
      { m_genCircle = genCircle; }


    public:
      /// Output helper for debugging
      friend std::ostream& operator<<(std::ostream& output, const CDCTrajectory2D& trajector2D)
      { return output << trajector2D.getGenCircle();  }



    private:
      GeneralizedCircle m_genCircle; ///< Memory for the generalized circle describing the trajectory
      Vector2D m_startPos2D; ///< Memory for the start position of the trajectory


      /// ROOT Macro to make CDCTrajectory2D a ROOT class.
      ClassDefInCDCLocalTracking(CDCTrajectory2D, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCTRAJECTORY2D_H
