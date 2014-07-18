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

#include <tracking/cdcLocalTracking/geometry/UncertainPerigeeCircle.h>

#include <tracking/cdcLocalTracking/topology/ISuperLayerType.h>

namespace Belle2 {

  namespace CDCLocalTracking {

    /// Particle trajectory as it is seen in xy projection
    class CDCTrajectory2D : public CDCLocalTracking::UsedTObject {

    public:

      /// Default constructor for ROOT compatibility.
      CDCTrajectory2D() : m_localOrigin(), m_localPerigeeCircle() {;}

      /// Constructs a trajectory from a generalized circle.
      /** Constructs a trajectory which is described by the given circle \n
       *  The start point is set to the closest approach to the origin */
      CDCTrajectory2D(const UncertainPerigeeCircle& perigeeCircle) :
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
      CDCTrajectory2D(const Vector2D& startPoint, const Vector2D& startMomentum, const FloatType& charge);

      /// Empty destructor
      ~CDCTrajectory2D() {;}

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
      FloatType calcPerpS(const Vector2D& point) const
      { return getLocalCircle().lengthOnCurve(Vector2D(0.0, 0.0), point - getLocalOrigin()); }


      /// Calculate the travel distance between the two given positions
      /** Returns the travel distance on the trajectory from the first given point to \n
       *  the second given point. This is subjected to a discontinuity at the far point \n
       *  of the circle. Hence the value return is in the range from -PI*radius to PI*radius \n
       *  If you have a heavily curling track you have care about the feasibility of this \n
       *  calculation. */
      FloatType calcPerpSBetween(const Vector2D& fromPoint, const Vector2D& toPoint) const
      { return getLocalCircle().lengthOnCurve(fromPoint - getLocalOrigin(), toPoint - getLocalOrigin()); }

      /// Gives the three dimensional point which is on the skew line as well as on the circle in the xy projection
      /** This method makes the reconstruction of the z coordinate possble by using the skewness \n
       *  stereo layer of the stereo wires.  The point is determined such that it is on the given \n
       *  skew line as well as on the circular trajector in the xy projection. */
      Vector3D reconstruct3D(const BoundSkewLine& skewLine) const;

    public:
      /// Calculates the closest approach on the trajectory to the global origin
      Vector2D getGlobalPerigee() const
      { return getGlobalCircle().perigee(); }

      /// Calculates the closest approach on the trajectory to the given point
      Vector2D getClosest(const Vector2D& point) const
      { return getLocalCircle().closest(point - getLocalOrigin()) + getLocalOrigin(); }

      /// Calculates the close point with the same polarR on the trajectory to the given point
      /** This returns the point where the trajectory reaches as certain distance from the origin \n
       *  ( in the xy projection ). It is useful to estimate where the trajectory reaches a  \n
       *  specific wire layer. */
      Vector2D getCloseSamePolarR(const Vector2D& point) const
      { return getGlobalCircle().samePolarR(point); }

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
      FloatType getMaximalPolarR() const
      { return  getGlobalCircle().maximalPolarR(); }

      /// Getter for the minimal distance from the origin - same as absolute value of the impact parameter
      FloatType getMinimalPolarR() const
      { return  getGlobalCircle().minimalPolarR(); }

      /// Getter for the signed impact parameter of the trajectory
      FloatType getGlobalImpact() const
      { return  getGlobalCircle().impact(); }

      /// Indicates if the trajectory is moving outwards or inwards (to or away from the origin) from the start point on
      bool isMovingOutward() const
      { return getStartUnitMom2D().dot(getLocalOrigin()) > 0; }

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

      /// Indicates the superlayer the trajectory starts in.
      ISuperLayerType getStartISuperLayer() const;

      /// Indicates the minimal superlayer the trajectory traverses
      ISuperLayerType getMinimalISuperLayer() const;

    public:
      /// Calculates the perpendicular travel distance from the last position of the fromEntity to the first position of the toEntity.
      /** Entities can be all eventdata/entities , segments and tracks **/
      template<class FromEntity, class ToEntity>
      FloatType getPerpSGap(const FromEntity& fromEntity, const ToEntity& toEntity) const {
        Vector2D fromRecoPos2D = fromEntity.getBackRecoPos2D(*this);
        Vector2D toRecoPos2D = toEntity.getFrontRecoPos2D(*this);
        return calcPerpSBetween(fromRecoPos2D, toRecoPos2D);
      }


      /// Calculates the perpendicular travel distance from the first position of the fromEntity to the first position of the toEntity.
      /** Entities can be all eventdata/entities , segments and tracks **/
      template<class FromEntity, class ToEntity>
      FloatType getPerpSFrontOffset(const FromEntity& fromEntity, const ToEntity& toEntity) const {
        Vector2D fromRecoPos2D = fromEntity.getFrontRecoPos2D(*this);
        Vector2D toRecoPos2D = toEntity.getFrontRecoPos2D(*this);
        return calcPerpSBetween(fromRecoPos2D, toRecoPos2D);
      }

      /// Calculates the perpendicular travel distance from the last position of the fromEntity to the last position of the toEntity.
      /** Entities can be all eventdata/entities , segments and tracks **/
      template<class FromEntity, class ToEntity>
      FloatType getPerpSBackOffset(const FromEntity& fromEntity, const ToEntity& toEntity) const {
        Vector2D fromRecoPos2D = fromEntity.getBackRecoPos2D(*this);
        Vector2D toRecoPos2D = toEntity.getBackRecoPos2D(*this);
        return calcPerpSBetween(fromRecoPos2D, toRecoPos2D);
      }

      /// Calculates the perpendicular travel distance from the first position of the entity to the last position of the entity.
      /** Entities can be all eventdata/entities , segments and tracks **/
      template<class Entity>
      FloatType getTotalPerpS(const Entity& entity) const {
        Vector2D frontRecoPos2D = entity.getFrontRecoPos2D(*this);
        Vector2D backRecoPos2D = entity.getBackRecoPos2D(*this);
        return calcPerpSBetween(frontRecoPos2D, backRecoPos2D);
      }

      /// Calculates if this trajectory and the entity are coaligned
      /** Returns the sign of the total perpendicular travel distance of the trajectory */
      template<class Entity>
      ForwardBackwardInfo isForwardOrBackwardTo(const Entity& entity) const {
        FloatType totalPerpS = getTotalPerpS(entity);
        if (totalPerpS > 0) return FORWARD;
        else if (totalPerpS < 0) return BACKWARD;
        else return UNKNOWN_INFO;
      }

      /// Extrapolate to the closest approach to the front of the first point of the entity
      template<class Entity>
      Vector2D getClosestToFront(const Entity& entity) const
      { return getClosest(entity.getFrontRecoPos2D()); }

      /// Extrapolate to the closest approach to the front of the last point of the entity
      template<class Entity>
      Vector2D getClosestToBack(const Entity& entity) const
      { return getClosest(entity.getBackRecoPos2D()); }

      /// Extrapolate to center of entity and calculate the perpendicular distance.
      template<class Entity>
      Vector2D getClosestToCenter(const Entity& entity) const
      { return getClosest(entity.getCenterRecoPos2D()); }



      /// Get distance from the trajectory to the center of the entity
      template<class Entity>
      FloatType getDist2DToCenter(const Entity& entity) const
      { return getDist2D(entity.getCenterRecoPos2D()); }

      /// Get distance from the trajectory to the front of the entity
      template<class Entity>
      FloatType getDist2DToFront(const Entity& entity) const
      { return getDist2D(entity.getFrontRecoPos2D()); }

      /// Get distance from the trajectory to the back of the entity
      template<class Entity>
      FloatType getDist2DToBack(const Entity& entity) const
      { return getDist2D(entity.getBackRecoPos2D()); }



      /// Get the unit momentum of the trajectory at the center of the entity
      template<class Entity>
      Vector2D getUnitMom2DAtCenter(const Entity& entity) const
      { return getUnitMom2D(entity.getCenterRecoPos2D()); }

      /// Get the unit momentum of the trajectory at the front of the entity
      template<class Entity>
      Vector2D getUnitMom2DAtFront(const Entity& entity) const
      { return getUnitMom2D(entity.getFrontRecoPos2D()); }

      /// Get the unit momentum of the trajectory at the back of the entity
      template<class Entity>
      Vector2D getUnitMom2DAtBack(const Entity& entity) const
      { return getUnitMom2D(entity.getBackRecoPos2D()); }


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
      FloatType getAbsMom2D() const;

      /// Get the momentum at the start point of the trajectory
      inline Vector2D getStartMom2D() const
      { return  getStartUnitMom2D() *= getAbsMom2D();  }

      /// Get the unit momentum at the start point of the trajectory
      inline Vector2D getStartUnitMom2D() const
      { return  getLocalCircle().tangential();  }


      //FloatType setStartPos2D(const Vector2D& point);

      /// Setter for start point and momentum at the start point subjected to the charge sign.
      void setPosMom2D(const Vector2D& pos2D, const Vector2D& mom2D, const FloatType& charge);



      /// Clears all information from this trajectoy
      void clear() {
        m_localOrigin.set(0.0, 0.0);
        m_localPerigeeCircle.setNull();
      }



    public:
      /// Getter for the circle in global coordinates
      GeneralizedCircle getGlobalCircle() const {
        // Down cast since we do not necessarily wont the covariance matrix transformed as well
        GeneralizedCircle result(m_localPerigeeCircle);
        result.passiveMoveBy(-getLocalOrigin());
        return result;
      }

      /// Setter for the generalized circle that describes the trajectory.
      void setGlobalCircle(const UncertainPerigeeCircle& perigeeCircle) {
        m_localPerigeeCircle = perigeeCircle;
        m_localPerigeeCircle.passiveMoveBy(getLocalOrigin());
      }


      /// Getter for the cirlce in local coordinates
      const UncertainPerigeeCircle& getLocalCircle() const
      { return m_localPerigeeCircle; }

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
      FloatType setLocalOrigin(const Vector2D& localOrigin) {
        FloatType result = calcPerpS(localOrigin);
        m_localPerigeeCircle.passiveMoveBy(localOrigin - m_localOrigin);
        m_localOrigin = localOrigin;
        // cout << "New projTravelDistanceRef " << m_projTravelDistanceRef << endl;
        return result;
      }



    public:
      /// Output helper for debugging
      friend std::ostream& operator<<(std::ostream& output, const CDCTrajectory2D& trajectory2D) {
        return output << "Local origin : " << trajectory2D.getLocalOrigin() <<  ", "
               << "local circle : " << trajectory2D.getLocalCircle();
      }


    private:
      /// Memory for local coordinate origin of the circle representing the trajectory in global coordinates
      Vector2D m_localOrigin;

      /// Memory for the generalized circle describing the trajectory in coordinates from the local origin
      UncertainPerigeeCircle m_localPerigeeCircle;

      /// Memory for the start position of the trajectory
      //Vector2D m_startPos2D;

      /// ROOT Macro to make CDCTrajectory2D a ROOT class.
      ClassDefInCDCLocalTracking(CDCTrajectory2D, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCTRAJECTORY2D_H
