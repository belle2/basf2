/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/geometry/SZParameters.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  A line in the sz space
     *
     *  The line is represented in sz parameters tan lambda and z0 beginning the
     *  slope and the intercept of the line in the sz space.
     */
    class SZLine {

    public:
      /// Default constructor for ROOT compatibility.
      SZLine()
        : m_tanLambda(NAN)
        , m_z0(NAN)
      {
      }

      /// Constructor from slope and intercept in the sz space
      SZLine(double tanLambda, double z0)
        : m_tanLambda(tanLambda)
        , m_z0(z0)
      {
      }

      /// Constructor from the sz parameters
      explicit SZLine(const SZParameters& szParameters)
        : m_tanLambda(szParameters(ESZParameter::c_TanL))
        , m_z0(szParameters(ESZParameter::c_Z0))
      {
      }

    public:
      /// Getter for the tan lambda parameter
      double tanLambda() const
      {
        return m_tanLambda;
      }

      /// Getter for the slope in the sz space which is equivalent to tan lambda
      double slope() const
      {
        return m_tanLambda;
      }

      /// Returns the slope over the second coordinate
      double inverseSlope() const
      {
        return 1 / tanLambda();
      }

      /// Getter for the theta dip  angle
      double theta() const
      {
        return m_tanLambda;
      }

      /// Getter for the tan lambda parameter
      void setTanLambda(double tanLambda)
      {
        m_tanLambda = tanLambda;
      }

      /// Getter for the z0 parameter
      double z0() const
      {
        return m_z0;
      }

      /// Getter for the intercept in the sz space which is equivalent to z0
      double intercept() const
      {
        return m_z0;
      }

      /// Getter for the z0 parameter
      void setZ0(double z0)
      {
        m_z0 = z0;
      }

      /// Getter for the sz parameters
      SZParameters szParameters() const
      {
        using namespace NSZParameterIndices;
        SZParameters result;
        result(c_TanL) = tanLambda();
        result(c_Z0) = z0();
        return result;
      }

      /// Returns the the two dimensional arc length s where the z coordinate approaches zero
      double zero() const
      {
        return -intercept() / slope();
      }

      /// Maps the two dimensional arc length s to z
      double map(double s) const
      {
        return tanLambda() * s + z0();
      }

      /// Maps the two dimensional arc length s to z
      double operator()(double s) const
      {
        return map(s);
      }

      /// Maps the z coordinate to the two dimensional arc length s
      double inverseMap(double z) const
      {
        return (z - z0()) / tanLambda();
      }

    public:
      /// Sets the parameters to a invalid representation
      void invalidate()
      {
        setZ0(NAN);
        setTanLambda(NAN);
      }

      /// Indicates if the line parameters do not represent a valid line
      bool isInvalid() const
      {
        return not std::isfinite(tanLambda()) or not std::isfinite(z0());
      }

      /// Reverse the arc length direction in place
      void reverse()
      {
        m_tanLambda = -m_tanLambda;
      }

      /// Returns a copy of the line with reversed the arc length direction.
      SZLine reversed() const
      {
        return SZLine(-tanLambda(), z0());
      }

    public:
      /**
       *  Calculates the signed distance of the point to the line.
       *  Returns the signed distance of the point to the line. The sign is positiv \n
       *  below the curve and negative above it.
       */
      double distance(const Vector2D& szPoint) const
      {
        return distance(szPoint.first(), szPoint.second());
      }

      /**
       *  Calculates the signed distance of the point given by its to coordinates to the line.
       *  Returns the signed distance of the point to the line. The sign is positiv \n
       *  below the curve and negative above it.
       */
      double distance(double s, double z) const
      {
        return (map(s) - z) / hypot2(1, tanLambda());
      }

      /// Calculates the intersection point of two line. Infinity for parallels
      Vector2D intersection(const SZLine& szLine) const;

      /// Passivelly move the coordinate system in place by the given sz vector
      void passiveMoveBy(const Vector2D& bySZ)
      {
        passiveMoveBy(bySZ.first(), bySZ.second());
      }

      /// Passivelly move the coordinate system in place by the given s and z coordinates
      void passiveMoveBy(double s, double z)
      {
        m_z0 = map(s) - z;
      }

      /// Return a line passivelly move by the given vector as a copy
      SZLine passiveMovedBy(const Vector2D& bySZ) const
      {
        return passiveMovedBy(bySZ.first(), bySZ.second());
      }

      /// Return a line passivelly move by the given s and z coordinates as a copy
      SZLine passiveMovedBy(double s, double z) const
      {
        return SZLine(tanLambda(), map(s) - z);
      }

      /// Computes the Jacobi matrix for a move of the coordinate system by the given vector.
      SZJacobian passiveMoveByJacobian(const Vector2D& bySZ) const
      {
        using namespace NSZParameterIndices;
        SZJacobian result = SZUtil::identity();
        result(c_Z0, c_TanL) = bySZ.first();
        return result;
      }

    private:
      /// Memory for the tan lambda parameter
      double m_tanLambda;

      /// Memory for the z0 parameter
      double m_z0;

    };
  }
}
