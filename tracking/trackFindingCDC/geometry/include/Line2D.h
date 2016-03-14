/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef LINE2D_H
#define LINE2D_H

#include <tracking/trackFindingCDC/rootification/SwitchableRootificationBase.h>
#include <tracking/trackFindingCDC/typedefs/BasicTypes.h>

#include <tracking/trackFindingCDC/geometry/Vector2D.h>


namespace Belle2 {
  namespace TrackFindingCDC {

    /// A two dimensional normal line
    /** This class represents a line in two dimensional space by its normal definition\n
     *  n0 + n1*x + n2*y = 0 subjected to n1*n1 + n2*n2 = 1 \n
     *  The vector (n1, n2) is called unit normal vector.
     *  It has an orientation. It is defining a right half plane with all points having \n
     *  n0 + n1*x + n2*y > 0 and a left half plane for point having n0 + n1*x + n2*y < 0 \n
     *  This naturally corresponds to a direction of positiv advance being the tangential \n
     *  to the line (-n2 ,n1) \n
     *  This line is best suited for linear least square fits. */
    class Line2D : public TrackFindingCDC::SwitchableRootificationBase {

    public:


      /// Default constructor for ROOT compatibility.
      Line2D(): m_n0(0.0), m_n12(0.0, 0.0) { ; }

      ///Constructs taking all three line parameters
      Line2D(const FloatType& n0, const FloatType& n1, const FloatType& n2):
        m_n0(n0), m_n12(n1, n2) { normalize(); }

      ///Constructs taking the distance to the origin ( n0 ) and the normal vector
      Line2D(const FloatType& n0, const Vector2D& n12):
        m_n0(n0), m_n12(n12) { normalize(); }

      ///Constructs a line from its slope and intercept over the first coordinate (default forward orientation).
      /** Constucts a line of the points fullfilling y = slope *x + intercept.
       *  The Line2D has an additional orientation in contrast to y = slope *x + intercept.
       *  The default orientation for the line is forward with the positiv x axes.
       */
      static Line2D fromSlopeIntercept(const FloatType& slope,
                                       const FloatType& intercept)
      { return Line2D(intercept, slope, -FORWARD); }

      /// Constructs a line from its slope and intercept over the first coordinate with the given orientation.
      /** Constucts a line of the points fullfilling y = slope *x + intercept.
       *  The Line2D has an additional orientation in contrast to y = slope *x + intercept.
       *  The forward backward info sets if the constructed line shall have the direction with increasing or \n
       *  decreasing x respectivly.
       */
      static Line2D fromSlopeIntercept(const FloatType& slope,
                                       const FloatType& intercept,
                                       const ForwardBackwardInfo& orientation)
      { return Line2D(intercept * orientation, slope * orientation, -orientation); }

      /// Constructs a line through the two given points
      static Line2D throughPoints(const Vector2D& start, const Vector2D& end)
      { return Line2D(end.cross(start), (start - end).orthogonal()); }


      /// Empty deconstructor
      ~Line2D() { ; }


    public:
      /// Getter for the first line parameter
      const FloatType& n0() const { return m_n0; }
      /// Getter for the second line parameter
      const FloatType& n1() const { return m_n12.first(); }
      /// Getter for the third line parameter
      const FloatType& n2() const { return m_n12.second(); }
      /// Getter for the unit normal vector to the line
      const Vector2D& n12() const { return m_n12; }

    public:
      /// Setter for first line parameter
      /** This sets the signed distance of the line to the origin.
       *  Its not involved in the normalization hence it can be  \n
       *  exported as a public function. */
      void setN0(const FloatType& n0) { m_n0 = n0; }

    private:
      ///Setter for the second line parameter. May violate the normlization.
      void setN1(const FloatType& n1) { m_n12.setFirst(n1); }
      ///Setter for the third line parameter. May violate the normlization.
      void setN2(const FloatType& n2) { m_n12.setSecond(n2); }

      ///Setter for the normal vector by its coordinates.
      void setN12(const FloatType& n1, const FloatType& n2) { m_n12.set(n1, n2);}
      ///Setter for the normal vector.
      void setN12(const Vector2D& n12) { m_n12.set(n12);  }

    public:
      ///Setter for all line parameters. Normalizes correctly.
      /** Setter the for the line parameters which takes care of the correct normalization of the normal vector. */
      void setN(const FloatType& n0, const FloatType& n1, const FloatType& n2)
      { setN0(n0) ; setN12(n1, n2); normalize();}

      /// Sets all line parameters to zero.
      void setNull()
      { setN(0.0, 0.0, 0.0); }

      /// Setter for the intercept and slope.
      /** Sets the new intercept and slope of the line the direction is set to be forward with the increasing x axes. */
      void setSlopeIntercept(const FloatType& slope, const FloatType& intercept)
      { setN0(intercept); setN1(slope); setN2(-1.0); normalize(); }


      /// Setter for the intercept and slope with explicit orientation
      /** Sets the new intercept and slope of the line the direction is set to be forward with the increasing x axes. */
      void setSlopeIntercept(const FloatType& slope, const FloatType& intercept, const ForwardBackwardInfo& orientation)
      { setN0(intercept * orientation); setN1(slope * orientation); setN2(-orientation); normalize(); }


      /// Updates the parameters to obey the normalization condition
      void normalize()
      {
        if (not isNull()) {
          scaleN(1.0 / normalization());
        }
      }

      /// Flips orientation the line in place
      void reverse() { scaleN(-1.0); }

      /// Returns a copy of the line with the reversed orientation
      Line2D reversed() const { return Line2D(-n0(), -n1(), -n2()); }

    private:
      /// Scales all parameters. Helper for normalize.
      void scaleN(const FloatType& factor) { m_n0 *= factor; m_n12 *= factor; }

      /// Calculates the squared normalization. Helper for normalize.
      FloatType normalizationSquared() const
      { return n12().normSquared(); }

      /// Calculates the normalization. Helper for normalize.
      FloatType normalization() const
      { return sqrt(normalizationSquared()); }

    public:
      /// Calculates the signed distance of the point to the line.
      /** Returns the signed distance of the point to the line. The sign is positiv \n
       *  for the right side of the line and negativ for the left side. */
      FloatType distance(const Vector2D& point) const
      { return n0() + point.dot(n12()); }

      /// Calculates the signed distance of the point given by its to coordinates to the line.
      /** Returns the signed distance of the point to the line. The sign is positiv \n
       *  for the right side of the line and negativ for the left side. */
      FloatType distance(const FloatType& first, const FloatType& second) const
      { return n0() + first * n1() + second * n2(); }

      /// Returns the distance to the origin
      /** The distance to the origin is equivalent to the first line parameter*/
      FloatType distanceToOrigin() const
      { return n0(); }

      /// Returns the absolute value of distance(point)
      FloatType absoluteDistance(const Vector2D& point) const
      { return fabs(distance(point)); }

      /// Return if the point given is right or left of the line
      RightLeftInfo isRightOrLeft(const Vector2D& point) const
      { return sign(distance(point)); }

      /// Return if the point given is left of the line
      inline bool isLeft(const Vector2D& rhs) const
      { return isRightOrLeft(rhs) == LEFT; }

      /// Return if the point given is right of the line
      inline bool isRight(const Vector2D& rhs) const
      { return isRightOrLeft(rhs) == RIGHT; }

      /// Calculates the point of closest approach on the line to the point
      Vector2D closest(const Vector2D& point) const
      {
        const FloatType closestParallel = -n0();
        const FloatType closestOrthgonal = point.unnormalizedOrthogonalComp(n12());
        return Vector2D(n12(), closestParallel, closestOrthgonal);
      }

      /// Returns the point closest to the origin
      Vector2D closestToOrigin() const { return n12() * (-n0()); }

      /// Calculates the length on the curve between two points
      /** Takes to two point to their closest approach to the origin \n
       *  and calculates the distance between them. The length is signed \n
       *  take relativ to the direction of positiv advance. */
      FloatType lengthOnCurve(const Vector2D& from, const Vector2D& to) const
      { return to.unnormalizedOrthogonalComp(n12()) - from.unnormalizedOrthogonalComp(n12()); }


      /// Indicates if all circle parameters are zero
      inline bool isNull() const
      { return n0() == 0.0 and n12().isNull(); }


      // Note : Maybe add the tangential ( Vector2D ) for homogenity to the circles



      /// Gives the tangential vector in the direction of positiv advance on the line
      Vector2D tangential() const { return normal().orthogonal(); }

      /// Getter for the unit normal vector of the line
      const Vector2D& normal() const { return n12(); }

      /// Getter for the gradient of the distance field
      const Vector2D& gradient() const { return n12(); }


      /// Getter for the support point of the line being the point closest to the origin
      Vector2D support() const { return closestToOrigin(); }

      /// Returns if the direction of positiv advance has a common component aligned or anti aligned with the first coordinate.
      ForwardBackwardInfo alignedWithFirst() const { return -sign(n2()); }

      /// Returns if the direction of positiv advance has a common component aligned or anti aligned with the second coordinate.
      ForwardBackwardInfo alignedWithSecond() const { return sign(n1()); }

      /// Calculates the intersection point of two line. Infinity for parallels
      Vector2D intersection(const Line2D& line) const;

      /** @name Transformations of the line */
      /**@{*/
      /// Activelly moves the line in the direction given in place by the vector given
      void moveBy(const Vector2D& by)
      { m_n0 -= by.unnormalizedParallelComp(n12()); }

      /// Activelly moves the line in the direction given in place along the first coordinate
      void moveAlongFirst(const FloatType& first)
      { m_n0 -= n1() * first; }

      /// Activelly moves the line in the direction given in place along the second coordinate
      void moveAlongSecond(const FloatType& second)
      { m_n0 -= n2() * second; }

      /// Return a copy of the line activally moved long the first coordinate
      Line2D movedAlongFirst(const FloatType& first) const
      { return Line2D(n0() - n1() * first, n1(), n2()); }

      /// Return a copy of the line activally moved long the first coordinate
      Line2D movedAlongSecond(const FloatType& second) const
      { return Line2D(n0() - n2() * second, n1(), n2()); }



      /// Passivelly move the coordinate system  in place by the given vector
      void passiveMoveBy(const Vector2D& by)
      { m_n0 += by.unnormalizedParallelComp(n12()); }

      /// Passively move the coordinate system in place along the first coordinate
      void passiveMoveAlongFirst(const FloatType& first)
      { m_n0 += n1() * first; }

      /// Passively move the coordinate system in place along the second coordinate
      void passiveMoveAlongSecond(const FloatType& second)
      { m_n0 += n2() * second; }

      /// Return a copy of the line passively moved long the first coordinate
      Line2D passiveMovedAlongFirst(const FloatType& first) const
      { return Line2D(n0() + n1() * first, n1(), n2()); }

      /// Return a copy of the line passively moved long the first coordinate
      Line2D passiveMovedAlongSecond(const FloatType& second) const
      { return Line2D(n0() + n2() * second, n1(), n2()); }



      /// Flips the first coordinate inplace (no difference between active and pasive)
      void flipFirst()
      { m_n12.flipFirst(); }

      /// Flips the first coordinate inplace (no difference between active and pasive)
      void flipSecond()
      { m_n12.flipSecond(); }

      /// Makes a copy of the line with the first coordinate flipped (no difference between active and pasive)
      Line2D flippedFirst() const
      { return Line2D(n0(), n12().flippedFirst()); }

      /// Makes a copy of the line with the second coordinate flipped (no difference between active and pasive)
      Line2D flippedSecond() const
      { return Line2D(n0(), n12().flippedSecond()); }
      /**@}*/



      /** @name Line as a function of the first coordinate
       */
      /**@{*/
      /// Returns the slope over the first coordinate
      FloatType slope() const { return - n1() / n2(); }

      /// Returns the slope over the second coordinate
      FloatType inverseSlope() const { return - n2() / n1(); }

      /// Returns the intercept over the first coordinate
      FloatType intercept() const { return - n0() / n2(); }

      /// Returns the root of the line
      FloatType zero() const { return -n0() / n1(); }

      /// Maps the first coordinate to the second
      FloatType map(const FloatType& first) const { return  -(n0() + n1() * first) / n2(); }
      /// Maps the first coordinate to the second
      FloatType operator()(const FloatType& first) const { return  map(first); }

      /// Maps the second coordinate to the first
      FloatType inverseMap(const FloatType& second) const { return  -(n0() + n2() * second) / n1(); }

      /// Turns the line function into its inverse function in place
      void invert() { m_n12.swapCoordinates(); reverse(); }

      /// Returns the inverse function line as a copy
      Line2D inverted() const { return Line2D(-n0(), -n2(), -n1()); }
      /**@}*/



    private:
      FloatType m_n0; ///< Memory for the first line parameter
      Vector2D  m_n12; ///< Memory for the second line parameter

    private:
      /// ROOT Macro to make Line2D a ROOT class.
      TRACKFINDINGCDC_SwitchableClassDef(Line2D, 1);

    }; //class
  } // namespace TrackFindingCDC
} // namespace Belle2
#endif // LINE2D
