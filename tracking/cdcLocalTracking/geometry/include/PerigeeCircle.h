/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef PERIGEECIRCLE_H
#define PERIGEECIRCLE_H

#include <cmath>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include "Vector2D.h"
#include "StandardOriginCircle2D.h"

#include "GeneralizedCircle.h"


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

    class PerigeeCircle : public GeneralizedCircle {

    public:
      /// Default constructor for ROOT compatibility.
      PerigeeCircle() {;}

      /// Constructor with the signed curvature as single parameter.
      PerigeeCircle(const FloatType& curvature) :
        m_curvature(curvature),
        m_tangentialPhi(copysign(PI / 2.0 , -curvature)),
        m_tangential(Vector2D(0.0, -sign(curvature))),
        m_impact(0)
      { cacheN(); }

      /// Constructor taking the perigee and a signed curvature, the polar angle of the flight direction at the perigee and the signed impact parameter.
      PerigeeCircle(const FloatType& curvature,
                    const FloatType& tangentialPhi,
                    const FloatType& impact) :
        m_curvature(curvature),
        m_tangentialPhi(tangentialPhi),
        m_tangential(Vector2D::Phi(tangentialPhi)),
        m_impact(impact)
      { cacheN(); }

      /// Empty destructor
      ~PerigeeCircle() {;}

    private:
      void cacheN()
      { GeneralizedCircle::setPerigeeParameters(curvature(), tangential(), impact()); }


    public:
      //operator GeneralizedCircle() const
      //{ return GeneralizedCircle(n0(), n12(), n3()); }

      /// Setter for the perigee parameters.
      inline void setPerigeeParameters(const FloatType& curvature,
                                       const Vector2D& tangential,
                                       const FloatType& impact) {
        m_curvature = curvature;
        m_tangential = tangential;
        m_tangentialPhi = tangential.phi();
        m_impact = impact;
        cacheN();
      }

      /// Setter for the perigee parameters.
      inline void setPerigeeParameters(const FloatType& curvature,
                                       const FloatType& tangentialPhi,
                                       const FloatType& impact) {
        m_curvature = curvature;
        m_tangential = Vector2D::Phi(tangentialPhi);
        m_tangentialPhi = tangentialPhi;
        m_impact = impact;
        cacheN();
      }

      /// Setter for the generalized circle parameters.
      inline void setN(const FloatType& n0,
                       const FloatType& n1,
                       const FloatType& n2,
                       const FloatType& n3) {

        GeneralizedCircle::setN(n0, n1, n2, n3);
        m_curvature = GeneralizedCircle::curvature();
        m_tangential = GeneralizedCircle::tangential();
        m_tangentialPhi = m_tangential.phi();
        m_impact = GeneralizedCircle::impact();
      }

      /// Setter for the generalized circle parameters.
      inline void setN(const GeneralizedCircle& generalizedCircle) {
        setN(generalizedCircle.n0(),
             generalizedCircle.n1(),
             generalizedCircle.n2(),
             generalizedCircle.n3());
      }


    public:
      /// Setter for signed curvature.
      inline void setCurvature(const FloatType& curvature) {
        m_curvature = curvature;
        cacheN();
      }

      ///Getter for the signed curvature.
      inline const FloatType& curvature() const
      { return m_curvature; }

      /// Sets the impact parameter of the circle.
      inline void setImpact(const FloatType& impact) {
        m_impact = impact;
        cacheN();
      }

      /// Gives the signed distance of the origin to the circle.
      inline FloatType impact() const
      { return m_impact; }

      /// Sets the polar angle of the direction of flight at the perigee.
      inline void setTangentialPhi(const FloatType& tangentialPhi) {
        m_tangentialPhi = tangentialPhi;
        m_tangential = Vector2D::Phi(tangentialPhi);
        cacheN();
      }

      /// Sets the unit direction of flight at the perigee
      inline void setTangential(const Vector2D& tangential) {
        m_tangentialPhi = tangential.phi();
        m_tangential = tangential.unit();
        cacheN();
      }

      /// Gets the polar angle of the direction of flight at the perigee
      inline FloatType tangentialPhi() const
      { return m_tangentialPhi; }

      /// Getter for the tangtial vector at the perigee
      inline Vector2D tangential() const
      { return m_tangential; }

      /// Getter for the tangtial vector at the perigee
      inline Vector2D tangential(const Vector2D& pos) const
      { return GeneralizedCircle::tangential(pos); }

      /// Getter for the perigee point
      inline Vector2D perigee() const
      { return tangential().orthogonal() * impact(); }

      /// Indicates if the generalized circle is actually a line
      //inline bool isLine() const
      //{ return curvature() == 0.0; }

      /// Indicates if the generalized circle is actually a circle
      //inline bool isCircle() const
      //{ return curvature() != 0.0; }


      /// Gives the signed radius of the circle. If it was a line this will be infinity
      inline FloatType radius() const
      { return 1 / curvature(); }

      /// Gives the signed radius of the circle. If it was a line this will be infinity
      inline FloatType absRadius() const
      { return fabs(radius()); }

      /// Gives the minimal polar r the circle reaches (unsigned)
      inline FloatType minimalPolarR() const
      { return fabs(impact()); }

      /// Gives the maximal polar r the circle reaches
      inline FloatType maximalPolarR() const
      { return fabs(impact() + 2 * radius()); }

      /// Gives the center of the circle. If it was a line both components will be infinity
      //inline Vector2D center() const
      //{ return tangential().orthogonal() * (impact() + radius()); }

      /** @name Generalized Circle parameters
       *  In close relation to the generalized circle we provide the four generalized circle parameters with the same meaning as in the GeneralizedCircle class
       */
      /**@{*/
      ///Getter for the first circle parameter
      //inline FloatType n0() const
      //{ return impact() * (impact() * curvature() / 2.0 + 1.0); }

      ///Getter for the second circle parameter
      //inline FloatType n1() const
      //{ return n12().first(); }

      ///Getter for the third circle parameter
      //inline FloatType n2() const
      //{ return n12().second(); }

      ///Getter for the second and third circle parameter which natuarally from a vector
      //inline Vector2D n12() const
      //{ return -tangential().orthogonal() * (1 + curvature() * impact()); }

      ///Getter for the fourth circle parameter
      //inline FloatType n3() const
      //{ return curvature() / 2.0; }
      /**@}*/

    public:
      /// Sets the signed curvature to zero
      inline void setNull() {
        setPerigeeParameters(0.0, 0.0, 0.0);
        GeneralizedCircle::setNull();
      }

      /// Indicates the signed curvature is zero
      inline bool isNull() const {
        return
          curvature() == 0.0 and
          tangentialPhi() == 0.0 and
          impact() == 0.0 and
          GeneralizedCircle::isNull();
      }


    private:
      /// Returns the polar angle associated with the reverse vector
      inline FloatType reversedPhi(const FloatType& phi) const
      { return phi > 0 ? phi - PI : phi + PI; }

    public:
      /// Flips the orientation of the circle in place
      inline void reverse() {
        m_curvature = -m_curvature;
        m_tangentialPhi = reversedPhi(m_tangentialPhi);
        m_tangential.reverse();
        m_impact = -m_impact;
        GeneralizedCircle::reverse();
      }

      /// Returns a copy of the circle with opposite orientation.
      inline PerigeeCircle reversed() const
      { return PerigeeCircle(-curvature(), reversedPhi(tangentialPhi()), -impact()); }


    public:
      /// Gradient of the distance field
      /**
       * Gives the gradient of the approximated distance field for the given point.
       * @param point Point in the plane to calculate the gradient
       * @return Gradient of the distance field
       */
      //inline Vector2D gradient(const Vector2D& point) const
      //{ m_generalizedCircle.gradient(point); }

      /// Normal vector to the circle near the given position
      /**
       * Gives the unit normal vector to the circle line.
       * It points to the outside of the circle for counterclockwise orientation,
       * and inwards for the clockwise orientation.
       * It is essentially the gradient normalized to unit length
       * @param point Point in the plane to calculate the tangential
       * @return Unit normal vector to the circle line
       */
      //inline Vector2D normal(const Vector2D& point) const
      //{ m_generalizedCircle.normal(point); }


      /// Tangential vector to the circle near the given position
      /**
       * Gives the unit tangential vector to the circle line.
       * It always points in the direction of positiv advance
       * at the point of closest approach from the given point.
       * @param point Point in the plane to calculate the tangential
       * @return Unit tangential vector to the circle line
       */
      //inline Vector2D tangential(const Vector2D& point) const
      //{ m_generalizedCircle.tangential(point); }

      ///Closest approach on the circle to the point
      /**
       * Calculates the closest point on the circle relative to the given point.
       * @param point Point in the plane to calculate the closest approach to
       * @return Point of closest approach on the circle.
       */
      //Vector2D closest(const Vector2D& point) const
      //{ m_generalizedCircle.closest(point); }


      /// Calculates if the to vector is closer to the from vector following the along orientation of the circle or against.
      /** Returns:
       *  * FORWARD in case the to vector is closer following the along the orientation
       *  * BACKWARD in case the to vector is closer against the orientation.
       *  * UNKNOWN_INFO if neither can be determined.
       */
      //ForwardBackwardInfo isForwardOrBackwardOf(const Vector2D& from, const Vector2D& to) const
      //{ return m_generalizedCircle.isForwardOrBackwardOf(from, to); }


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
      //Vector2D chooseNextForwardOf(const Vector2D& start, const Vector2D& end1, const Vector2D& end2) const
      //{ return m_generalizedCircle.chooseNextForwardOf(start, end1, end2); }

      /// Calculates the two points with the given polar radius on the generalised circle
      //std::pair<Belle2::CDCLocalTracking::Vector2D, Belle2::CDCLocalTracking::Vector2D> samePolarR(const FloatType& polarR) const
      //{ return m_generalizedCircle.samePolarR(polar); }

      ///Approach with the same polar radius on the circle to the point
      /**
       * Calculates the point on the circle with the same polar radius as the given point.
       * This is particularly useful to extraplotate within  a certain layer.
       * In case on intersection with this polar radius exists the function return Vector2D(NAN,NAN)
       * @param point Point in the plane to calculate the same polar radius point on the circle
       * @return Close point with same polar radius on the circle
       */
      //Vector2D samePolarR(const Vector2D& point) const
      //{ return m_generalizedCircle.samePolarR(point); }

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
      //Vector2D samePolarRForwardOf(const Vector2D& startPoint, const FloatType& polarR) const
      //{ return m_generalizedCircle.samePolarRForwardOf(startPoint, polarR); }

      /// Approximate distance
      /**
       * Gives a fast signed approximation of the distance to the circle.
       * The absolute value of the fast distance is accurate up to first
       * order of distance/circle radius, hence best used with big circles and small distances.
       * The sign of the fast distance indicates if the point is to the right or to the left of the circle.
       */
      //inline FloatType fastDistance(const Vector2D& point) const
      //{ return m_generalizedCircle.fastDistance(point); }

      /// Approximate distance to the origin
      //inline FloatType fastImpact() const
      //{ return m_generalizedCircle.fastImpact(); }


      /// Gives the proper distance of the point to the circle line retaining the sign of the fast distance.
      //FloatType distance(const Vector2D& point) const;
      //{ return m_generalizedCircle.distance(point); }


      /// Gives the proper absolute distance of the point to the circle line.
      //inline FloatType absDistance(const Vector2D& point) const
      //{ return fabs(distance(point)); }

      /// Indicates if the point is on the right or left side of the circle. This is also refered to as alpha.
      //RightLeftInfo isRightOrLeft(const Vector2D& point) const
      //{ return sign(fastDistance(point)); }

      /// Return if the point given is left of the line
      //inline bool isLeft(const Vector2D& rhs) const
      //{ return isRightOrLeft(rhs) == LEFT; }

      /// Return if the point given is right of the line
      //inline bool isRight(const Vector2D& rhs) const
      //{ return isRightOrLeft(rhs) == RIGHT; }


      /// Gives the orientation of the circle
      /**
       * The circle can be either orientated counterclockwise or clockwise.
       * @return CCW for counterclockwise travel, CW for clockwise.
       */
      //inline CCWInfo orientation() const
      //{  return sign(curvature()); }

      /// Calculates the angle between two points of closest approach on the circle
      /**
       * The angle is signed positiv for a counterclockwise rotation.
       * The points are essentially first taken to their closest approach
       * before we take the opening angle as seen from the circle center.
       * The angle will zero if the generalized circle was line.
       */
      //inline FloatType openingAngle(const Vector2D& from, const Vector2D& to) const
      //{ return gradient(from).angleWith(gradient(to)); } //not optimal in number of computations

      /// Calculates the arc length between two points of closest approach on the circle.
      /**
       * The arc length is signed positiv for travel in orientation direction.
       * In the circle case the arc length is between -PI*radius and PI*radius,
       * hence the discontinuity is on the far side of the circle relative to the given from point.
       * The points are essentially first taken to their closest approach
       * before we take the length on the curve.
       * For the line case the length is the distance component parallel to the line.
       */
      //FloatType lengthOnCurve(const Vector2D& from, const Vector2D& to) const;


      /// Moves the coordinates system by the given vector. Updates perigee parameters in place
      void passiveMoveBy(const Vector2D& by);


    public:
      /// Debug helper
      friend std::ostream& operator<<(std::ostream& output, const PerigeeCircle& perigeeCircle) {
        return output <<
               "PerigeeCircle(" <<
               "curvature=" << perigeeCircle.curvature() << "," <<
               "tangentialPhi=" << perigeeCircle.tangentialPhi() << "," <<
               "impact=" << perigeeCircle.impact() << ")" ;
      }


    private:
      FloatType m_curvature; ///< Memory for the signed curvature
      FloatType m_tangentialPhi; ///< Memory for the polar angle of the direction of flight at the perigee
      Vector2D m_tangential; ///< Cached unit direction of flight at the perigee

      FloatType m_impact; ///< Memory for the signed impact parameter

      /// ROOT Macro to make StandardOriginCircle2D a ROOT class.
      ClassDefInCDCLocalTracking(PerigeeCircle, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // PERIGEECIRCLE
