/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef STANDARDORIGINCIRCLE_H
#define STANDARDORIGINCIRCLE_H

#include <cmath>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include "Vector2D.h"
#include "Line2D.h"
#include "Circle2D.h"
#include "BoundSkewLine.h"

namespace Belle2 {

  namespace CDCLocalTracking {

    ///A circle through the origin which center lies on the positive x-axis
    /** Makes a smooth generalization from a two dimensional normal line ( like Line2D ) to a circle \n
     *  since its single parameter is its curvature. This may serve as a base class for other low curvature
     *  circles, because the general case is similar the the standard circle up to translational and rotational
     *  transformations.
     *  This circle is implemented as a parameter curve. The parameter along the curve is the distance on the
     *  circle from the origin. It can carry an orientation in the sign of the curvature.
     */
    class StandardOriginCircle2D : public UsedTObject {

    public:

      /// Default constructor for ROOT compatibility.
      StandardOriginCircle2D() {;}

      /// Constructor with the signed curvature as single parameter
      StandardOriginCircle2D(const FloatType& signedCurvature) : m_signedCurvature(signedCurvature) {;}

      /// Empty destructor
      ~StandardOriginCircle2D() {;}

    private:
      ///Setter for first circle parameter. Makes _no_ normalization after setting. Use is discouraged.
      inline void setSignedCurvature(const FloatType& signedCurvature) { m_signedCurvature = signedCurvature; }

    public:
      ///Getter for the signed curvature
      inline const FloatType& signedCurvature() const { return m_signedCurvature; }

      ///Getter for the absolute value of curvature
      inline FloatType curvature() const { return fabs(m_signedCurvature); }


      /** @name Generalized Circle parameters
       *  In close relation to the generalized circle we provide the four generalized circle parameters with the same meaning as in the GeneralizedCircle class
       */
      /**@{*/
      ///Getter for the first circle parameter
      inline FloatType n0() const { return 0.0; }

      ///Getter for the second circle parameter
      inline FloatType n1() const { return -sign(signedCurvature()) * 1.0; }
      ///Getter for the third circle parameter
      inline FloatType n2() const { return 0.0; }

      ///Getter for the second and third circle parameter which natuarally from a vector
      inline Vector2D n12() const { return Vector2D(n1(), n2()); }
      ///Getter for the fourth circle parameter
      inline FloatType n3() const { return signedCurvature() / 2.0; }
      /**@}*/

      /// Sets the signed curvature to zero
      void setNull()
      { setSignedCurvature(0.0); }

      /// Indicates the signed curvature is zero
      inline bool isNull() const
      { return signedCurvature() == 0; }

      /// Flips the orientation of the circle in place
      inline void reverse() { m_signedCurvature = -m_signedCurvature; }

      /// Returns a copy of the circle with opposite orientation.
      inline StandardOriginCircle2D reversed() const
      { return StandardOriginCircle2D(-signedCurvature()); }

      /** Returns a copy of the circle in conformal space.
       *  Origin circles in normal space map to straight lines in conformal space.*/
      inline Line2D conformalTransformed() const
      { return Line2D(n3(), n12()); }

    public:
      /// Gradient of the distance field
      /**
       * Gives the gradient of the approximated distance field for the given point.
       * @param point Point in the plane to calculate the gradient
       * @return Gradient of the distance field
       */
      inline Vector2D gradient(const Vector2D& point) const {
        Vector2D result = point * (2.0 * n3());
        result += n12();
        return result;
      }

      /// Normal vector to the circle near the given position
      /**
       * Gives the unit normal vector to the circle line.
       * It points to the outside of the circle for counterclockwise orientation,
       * and inwards for the clockwise orientation.
       * It is essentially the gradient normalized to unit length
       * @param point Point in the plane to calculate the tangential
       * @return Unit normal vector to the circle line
       */
      inline Vector2D normal(const Vector2D& point) const {
        Vector2D grad = gradient(point);
        grad.normalize();
        return grad;
      }

      /// Tangential vector to the circle near the given position
      /**
       * Gives the unit tangential vector to the circle line.
       * It always points in the direction of positiv advance
       * at the point of closest approach from the given point.
       * @param point Point in the plane to calculate the tangential
       * @return Unit tangential vector to the circle line
       */
      inline Vector2D tangential(const Vector2D& point) const { return normal(point).orthogonal(); }

      ///Closest approach on the circle to the point
      /**
       * Calculates the closest point on the circle relative to the given point.
       * @param point Point in the plane to calculate the closest approach to
       * @return Point of closest approach on the circle.
       */
      Vector2D closest(const Vector2D& point) const;

      /// Calculates the closest approach to the two dimensional origin
      Vector2D closestToOrigin() const { return Vector2D(0.0, 0.0); }


      /// Returns the end point which is first reached if one follows the forward direction of the circle starting from the start point
      /**
       * Evaluates which of the given end points end1 and end2 is closer to start
       * This espicially treats the discontinuity on the far side of the circle correctly.
       * If the generalized circle is truely a line none of the points might lie in the forward direction and Vector2D(NAN,NAN) is returned
       * @param start Point to start the traversal
       * @param end1 One possible end point
       * @param end2 Other possible end point
       * @return end1 or end2 depending, which lies closer to start in the forward direction or Vector2D(NAN,NAN) if neither end1 nor end2 are reachable in the forward direction (line case only)
       */
      Vector2D chooseNextForwardOf(const Vector2D& start, const Vector2D& end1, const Vector2D& end2) const;

      /// Calculates the two points with the given polar radius on the generalised circle
      std::pair<Belle2::CDCLocalTracking::Vector2D, Belle2::CDCLocalTracking::Vector2D> samePolarR(const FloatType& polarR) const;

      ///Approach with the same polar radius on the circle to the point
      /**
       * Calculates the point on the circle with the same polar radius as the given point.
       * This is particularly useful to extraplotate within  a certain layer.
       * In case on intersection with this polar radius exists the function return Vector2D(NAN,NAN)
       * @param point Point in the plane to calculate the same polar radius point on the circle
       * @return Close point with same polar radius on the circle
       */
      Vector2D samePolarR(const Vector2D& point) const;

      ///Approach on the circle with the given polar radius that lies in the forward direction of a start point
      /**
       * Calculates the point on the circle with polar radius polarR,
       * which is closest following the circle in the direction of positive forward orientation
       * This is particularly useful to extraplotate into a certain layer.
       * In case on intersection with this polar radius exists the function return Vector2D(NAN,NAN)
       * @param startPoint Start point from which to follow in the circle in the forward direction
       * @param polarR Polar radius of interest
       * @return Close point in forward direction with same polar radius on the circle.
       */
      Vector2D samePolarRForwardOf(const Vector2D& startPoint, const FloatType& polarR) const;

      /// Approximate distance
      /**
       * Gives a fast signed approximation of the distance to the circle.
       * The absolute value of the fast distance is accurate up to first
       * order of distance/circle radius, hence best used with big circles and small distances.
       * The sign of the fast distance indicates if the point is to the right or to the left of the circle.
       */
      inline FloatType fastDistance(const Vector2D& point) const
      { return n0() + point.dot(n12()) + point.normSquared() * n3(); }

      /// Approximate distance to the origin
      inline FloatType fastDistanceToOrigin() const
      { return n0(); }

      /// Gives the proper distance of the point to the circle line retaining the sign of the fast distance.
      FloatType distance(const Vector2D& point) const;

      /// Gives the proper distance of the orign to the circle line retaining the sign of the fast distance.
      inline FloatType distanceToOrigin() const
      { return 0.0; }

      /// Gives the proper absolute distance of the point to the circle line.
      inline FloatType absoluteDistance(const Vector2D& point) const
      { return fabs(distance(point)); }

      /// Indicates if the point is on the right or left side of the circle. This is also refered to as alpha.
      RightLeftInfo isRightOrLeft(const Vector2D& point) const
      { return sign(fastDistance(point)); }

      /// Return if the point given is left of the line
      inline bool isLeft(const Vector2D& rhs) const
      { return isRightOrLeft(rhs) == LEFT; }

      /// Return if the point given is right of the line
      inline bool isRight(const Vector2D& rhs) const
      { return isRightOrLeft(rhs) == RIGHT; }

      /// Indicates if the generalized circle is actually a line
      inline bool isLine() const { return signedCurvature() == 0.0; }
      /// Indicates if the generalized circle is actually a circle
      inline bool isCircle() const { return signedCurvature() != 0.0; }

      /// Gives the radius of the circle. If it was a line this will be infinity
      inline FloatType radius() const { return 1 / curvature(); }

      /// Gives the signed radius of the circle. If it was a line this will be infinity
      inline FloatType signedRadius() const { return 1 / signedCurvature(); }

      /// Gives the center of the circle. If it was a line both components will be infinity
      inline Vector2D center() const
      { return Vector2D(radius(), 0); }

      /// Gives the orientation of the circle
      /**
       * The circle can be either orientated counterclockwise or clockwise.
       * @return CCW for counterclockwise travel, CW for clockwise.
       */
      inline CCWInfo orientation() const
      {  return sign(signedCurvature()); }

      /// Calculates the angle between two points of closest approach on the circle
      /**
       * The angle is signed positiv for a counterclockwise rotation.
       * The points are essentially first taken to their closest approach
       * before we take the opening angle as seen from the circle center.
       * The angle will zero if the generalized circle was line.
       */
      inline FloatType openingAngle(const Vector2D& from, const Vector2D& to) const
      { return gradient(from).angleWith(gradient(to)); } //not optimal in number of computations

      /// Calculates the arc length between two points of closest approach on the circle.
      /**
       * The arc length is signed positiv for travel in orientation direction.
       * In the circle case the arc length is between -PI*radius and PI*radius,
       * hence the discontinuity is on the far side of the circle relative to the given from point.
       * The points are essentially first taken to their closest approach
       * before we take the length on the curve.
       * For the line case the length is the distance component parallel to the line.
       */
      inline FloatType lengthOnCurve(const Vector2D& from, const Vector2D& to) const {
        return isLine() ?
               to.fastOrthogonalComp(n12()) - from.fastOrthogonalComp(n12()) : // use fastParallelComp here because n12() is of unit length in the n3 == 0 zero case
               openingAngle(from, to) / 2 / n3();
      }

      /// Debug helper
      friend std::ostream& operator<<(std::ostream& output, const StandardOriginCircle2D& standardOriginCircle2D)
      { return output << "StandardOriginCircle2D with signed curvature = " << standardOriginCircle2D.signedCurvature(); }


    private:
      FloatType m_signedCurvature;

      /// ROOT Macro to make StandardOriginCircle2D a ROOT class.
      ClassDefInCDCLocalTracking(StandardOriginCircle2D, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // STANDARDORIGINCIRCLE
