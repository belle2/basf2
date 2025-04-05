/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ROOT headers. */
#include <Math/Vector3D.h>
#include <Math/Vector2D.h>
#include <Math/VectorUtil.h>
#include <TVector3.h>

namespace Belle2 {

  /**
   * Helper function to convert XYZVector to TVector3
   */
  static constexpr auto XYZToTVector = [](const ROOT::Math::XYZVector& a)
  {
    return TVector3(a.X(), a.Y(), a.Z());
  };

  namespace VectorUtil {

    /**
     * Set vector by polar coordinates.
     * @param[out] vector Vector.
     * @param[in]  mag    Magnitude.
     * @param[in]  theta  Polar angle.
     * @param[in]  phi    Azimuthal angle.
     */
    inline void setMagThetaPhi(ROOT::Math::XYZVector& vector,
                               double mag, double theta, double phi)
    {
      const double amag = std::abs(mag);
      const double sinTheta = std::sin(theta);
      const double x = amag * sinTheta * std::cos(phi);
      const double y = amag * sinTheta * std::sin(phi);
      const double z = amag * std::cos(theta);
      vector.SetXYZ(x, y, z);
    }

    /**
     * Set vector magnitude mag
     * @param[inout] vector Vector
     * @param[in]    mag    Magnitude
     */
    inline void setMag(ROOT::Math::XYZVector& vector, double mag)
    {
      setMagThetaPhi(vector, mag, vector.Theta(), vector.Phi());
    }

    /**
     * Set vector azimuthal angle theta
     * @param[inout] vector Vector
     * @param[in]    theta  Azimuthal angle
     */
    inline void setTheta(ROOT::Math::XYZVector& vector, double theta)
    {
      setMagThetaPhi(vector, vector.R(), theta, vector.Phi());
    }

    /**
     * Set vector polar angle phi
     * @param[inout] vector Vector
     * @param[in]    phi    Polar angle
     */
    inline void setPhi(ROOT::Math::XYZVector& vector, double phi)
    {
      setMagThetaPhi(vector, vector.R(), vector.Theta(), phi);
    }

    /**
     * Set vector by polar coordinates.
     * @param[out] vector Vector.
     * @param[in]  pt     Magnitude in xy-plane.
     * @param[in]  theta  Polar angle.
     * @param[in]  phi    Azimuthal angle.
     */
    inline void setPtThetaPhi(ROOT::Math::XYZVector& vector,
                              double pt, double theta, double phi)
    {
      const double aPt = std::abs(pt);
      const double x = aPt * std::cos(phi);
      const double y = aPt * std::sin(phi);
      const double tanTheta = std::tan(theta);
      const double z = tanTheta ? aPt / tanTheta : 0;
      vector.SetXYZ(x, y, z);
    }

    /**
     * Find CosTheta Angle between two generic 3D vectors
     * @param[in] v1  Vector v1
     * @param[in] v2  Vector v2
     * \return   cosine of Angle between the two vectors
     * \f[ \cos \theta = \frac { \vec{v1} \cdot \vec{v2} }{ | \vec{v1} | | \vec{v2} | } \f]
     *
     * Similar to the implementation in the ROOT documentation in Math::VectorUtil:
     * https://root.cern/doc/master/GenVector_2VectorUtil_8h_source.html#l00142
     * but that does not use the Mag2 method nor the Dot method to calculate the two properties
     * as it expects two different vector types and would only work for 3-vectors
     */
    template <class Vector>
    double CosTheta(const Vector&  v1, const Vector& v2)
    {
      double arg = 0.;
      double v1_r2 = v1.Mag2();
      double v2_r2 = v2.Mag2();
      double ptot2 = v1_r2 * v2_r2;
      if (ptot2 == 0) {
        return 0.0;
      }
      double pdot = v1.Dot(v2);
      arg = pdot / std::sqrt(ptot2);
      if (arg >  1.0) arg =  1.0;
      if (arg < -1.0) arg = -1.0;
      return arg;
    }


    /**
     * Find CosTheta Angle between two generic 3D vectors
     * @param[in] v1  Vector v1
     * @param[in] v2  Vector v2
     * \return   Angle between the two vectors
     * \f[ \theta = \cos ^{-1} \frac { \vec{v1} \cdot \vec{v2} }{ | \vec{v1} | | \vec{v2} | } \f]
     *
     * Same implementation as in the ROOT documentation in Math::VectorUtil:
     * https://root.cern/doc/master/GenVector_2VectorUtil_8h_source.html#l00169
     */
    template <class Vector>
    double Angle(const Vector&  v1, const Vector& v2)
    {
      return std::acos(CosTheta(v1, v2));
    }

    /** Calculates the part of this vector that is parallel to the given vector
     *  Adapted from tracking/trackingUtilities/geometry/Vector2D:
     *  https://gitlab.desy.de/belle2/software/basf2/-/blob/main/tracking/trackingUtilities/geometry/include/Vector2D.h?ref_type=heads#L445
     *
     *  Vector2D orthogonalVector(const Vector2D& relativTo) const
     * {
     *   return relativTo.scaled(relativTo.cross(*this) / relativTo.normSquared()).orthogonal();
     * }
     * with v2 = relativTo and v1 = *this.
     *
     * TODO: check whether this can be replaced by ROOT::Math::VectorUtil::PerpVector()
     * https://root.cern/doc/v628/namespaceROOT_1_1Math_1_1VectorUtil.html#a7503c235c5e71f762d44d253e5f6e9a3
     * though likely not for the 2D case as we use here.
     * If we can use it, obviously replace the function and where it's used.
     */
    inline ROOT::Math::XYVector orthogonalVector(const ROOT::Math::XYVector& v1, const ROOT::Math::XYVector& v2)
    {
      const double cross = v1.X() * v2.Y() - v1.Y() - v2.X();     // = relativTo.cross(*this)
      const ROOT::Math::XYVector tmp = v2 * (cross / v2.Mag2());  // = relativTo.scaled(cross / relativTo.normSquared())
      return ROOT::Math::XYVector(-tmp.Y(), tmp.X());             // = .orthogonal()
    }

    /** Calculates the part of this vector that is parallel to the given vector
     *  Adapted from tracking/trackingUtilities/geometry/Vector3D:
     *  https://gitlab.desy.de/belle2/software/basf2/-/blob/main/tracking/trackingUtilities/geometry/include/Vector3D.h?ref_type=heads#L442
     *
     * Vector3D parallelVector(const Vector3D& relativTo) const
     * {
     *   return relativTo.scaled(relativTo.dot(*this) / relativTo.normSquared());
     * }
     * with v2 = relativTo and v1 = *this.
     *
     * TODO: check whether this can be replaced by ROOT::Math::VectorUtil::ProjVector()
     * https://root.cern/doc/v628/namespaceROOT_1_1Math_1_1VectorUtil.html#ad067df491cad5594499d9a1860a00f4c
     * and if so, obviously replace where it is used.
     */
    inline ROOT::Math::XYZVector parallelVector(const ROOT::Math::XYZVector& v1, const ROOT::Math::XYZVector& v2)
    {
      const double dotp = v1.Dot(v2);                           // = relativTo.dot(*this)
      const ROOT::Math::XYZVector tmp = v2 * (dotp / v2.Mag2());  // = relativTo.scaled(dotp / relativTo.normSquared())
      return tmp;
    }

    /// Calculates the component orthogonal to the given vector
    /** The orthogonal component is the rest of the vector not parallel to \n
     *  relative to. Since the three dimensional space does not have a unique \n
     *  orientation given by the vector relative to, the sign of the orthogonal \n
     *  component is meaningless and is always set to positive
     *
     *  Adapted from tracking/trackingUtilities/geometry/Vector3D:
     *  https://gitlab.desy.de/belle2/software/basf2/-/blob/main/tracking/trackingUtilities/geometry/include/Vector3D.h?ref_type=heads#L436
     *
     * Vector3D orthogonalComp(const Vector3D& relativTo) const
     * {
     *   return relativTo.cross(*this).norm() / relativTo.norm();
     * }
     * with v2 = relativTo and v1 = *this.
     *
     * TODO: check whether this can be replaced by ROOT::Math::VectorUtil::Perp()
     * https://root.cern/doc/v628/namespaceROOT_1_1Math_1_1VectorUtil.html#a1dc4268cc95f8121f1833e24175b0ea2
     * and if so, obviously replace where it is used.
     */
    inline double orthogonalComp(const ROOT::Math::XYZVector& v1, const ROOT::Math::XYZVector& v2)
    {
      return v2.Cross(v1).R() / v2.R();
    }

  }

}
