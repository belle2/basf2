/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *               Eugenio Paoloni                                          *
 *               Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once


#include <framework/logging/Logger.h>

#include <TVector3.h>
#include <string>
#include <iostream>     // std::cout, std::fixed
#include <iomanip>      // std::setprecision
#include <typeinfo>
#include <cmath>


namespace Belle2 {

  /** A fast and root compatible alternative to TVector3.
   *
   * Goals:
   * - vectorizable
   * - root compatible
   * - fixed size
   * - featureset comparable to TVector3 (long term goal)
   * - interface/member functions compatible to TVector3
   *
   * DataType: shall be the data type one wants to store in the B2Vector (e.g. double)
   */

  template<typename DataType>
  class B2Vector3 {
  protected:
    /** Make sure that we only have floating point vectors */
    static_assert(std::is_floating_point<DataType>::value, "B2Vector3 only works with floating point types");
    /** contains the coordinates in given data type */
    DataType m_coordinates[3];
  public:
    /** storage type of the vector */
    typedef DataType value_type;

    /** empty Constructor sets everything to 0 */
    B2Vector3(void) : m_coordinates {static_cast<DataType>(0), static_cast<DataType>(0), static_cast<DataType>(0)} {};
    /** Constructor expecting 3 coordinates */
    B2Vector3(const DataType xVal, const DataType yVal, const DataType zVal): m_coordinates {xVal, yVal, zVal} {};
    /** Constructor using a reference */
    explicit B2Vector3(const DataType(& coords)[3]): m_coordinates {coords[0], coords[1], coords[2]} {};
    /** Constructor using a pointer */
    explicit B2Vector3(const DataType(* coords)[3]): m_coordinates {(*coords)[0], (*coords)[1], (*coords)[2]} {};
    /** Constructor expecting a TVector3 */
    // cppcheck-suppress noExplicitConstructor
    B2Vector3(const TVector3& tVec3): m_coordinates {static_cast<DataType>(tVec3.X()), static_cast<DataType>(tVec3.Y()), static_cast<DataType>(tVec3.Z())} {};
    /** Constructor expecting a pointer to a TVector3 */
    // cppcheck-suppress noExplicitConstructor
    B2Vector3(const TVector3* tVec3): m_coordinates {static_cast<DataType>(tVec3->X()), static_cast<DataType>(tVec3->Y()), static_cast<DataType>(tVec3->Z())} {};
    /** Constructor expecting a B2Vector3 of same type */
    explicit B2Vector3(const B2Vector3<DataType>& b2Vec3): m_coordinates {b2Vec3.X(), b2Vec3.Y(), b2Vec3.Z()} {};
    /** Constructor expecting a pointer to a B2Vector3 */
    explicit B2Vector3(const B2Vector3<DataType>* b2Vec3): m_coordinates {b2Vec3->X(), b2Vec3->Y(), b2Vec3->Z()} {};
    /** Constructor expecting a B2Vector3 of different type*/
    template <typename OtherType> B2Vector3(const B2Vector3<OtherType>& b2Vec3):
      m_coordinates {static_cast<DataType>(b2Vec3.X()), static_cast<DataType>(b2Vec3.Y()), static_cast<DataType>(b2Vec3.Z())} {};
    /** Constructor expecting a pointer to a B2Vector3 of different type */
    template <typename OtherType> explicit B2Vector3(const B2Vector3<OtherType>* b2Vec3):
      m_coordinates {static_cast<DataType>(b2Vec3->X()), static_cast<DataType>(b2Vec3->Y()), static_cast<DataType>(b2Vec3->Z())} {};

    /** member access without boundary check */
    DataType operator()(unsigned i) const { return m_coordinates[i]; }
    /** member access without boundary check */
    DataType operator[](unsigned i) const { return m_coordinates[i]; }
    /** member access without boundary check */
    DataType& operator()(unsigned i) { return m_coordinates[i]; }
    /** member access without boundary check */
    DataType& operator[](unsigned i) { return m_coordinates[i]; }

    /** Assignment via B2Vector3 */
    B2Vector3<DataType>& operator = (const B2Vector3<DataType>& b);
    /** Assignment via TVector3 */
    B2Vector3<DataType>& operator = (const TVector3& b);

    /** type conversion in TVector3 */
    operator TVector3() const { return GetTVector3(); }

    /** Comparison for equality with a B2Vector3 */
    bool operator == (const B2Vector3<DataType>& b) const { return X() == b.X() && Y() == b.Y() && Z() == b.Z(); }
    /** Comparison for equality with a TVector3 */
    bool operator == (const TVector3& b) const { return X() == b.X() && Y() == b.Y() && Z() == b.Z(); }
    /** Comparison != with a B2Vector3 */
    bool operator != (const B2Vector3<DataType>& b) const { return !(*this == b); }
    /** Comparison != with a TVector3 */
    bool operator != (const TVector3& b) const { return !(*this == b); }

    /** addition */
    B2Vector3<DataType>& operator += (const B2Vector3<DataType>& b);
    /** subtraction */
    B2Vector3<DataType>& operator -= (const B2Vector3<DataType>& b);
    /** scaling with real numbers */
    B2Vector3<DataType>& operator *= (DataType a);
    /** unary minus */
    B2Vector3<DataType> operator - () const { return B2Vector3<DataType>(-X(), -Y(), -Z()); }
    /**  Addition of 3-vectors. */
    B2Vector3<DataType> operator + (const B2Vector3<DataType>& b) const
    {
      return B2Vector3<DataType>(X() + b.X(), Y() + b.Y(), Z() + b.Z());
    }
    /**  Subtraction of 3-vectors */
    B2Vector3<DataType> operator - (const B2Vector3<DataType>& b) const
    {
      return B2Vector3<DataType>(X() - b.X(), Y() - b.Y(), Z() - b.Z());
    }
    /**  Scaling of 3-vectors with a real number */
    B2Vector3<DataType> operator * (DataType a) const
    {
      return B2Vector3<DataType>(a * X(), a * Y(), a * Z());
    }
    /** Scaling of 3-vectors with a real number */
    B2Vector3<DataType> operator / (DataType a) const
    {
      return B2Vector3<DataType>(X() / a, Y() / a, Z() / a);
    }
    /**  Scalar product of 3-vectors. */
    DataType operator * (const B2Vector3<DataType>& b) const { return Dot(b); }


    /**  The azimuth angle. returns phi from -pi to pi  */
    DataType Phi() const { return X() == 0 && Y() == 0 ? 0 : atan2(Y(), X()); }
    /**  The polar angle. */
    DataType Theta() const { return X() == 0 && Y() == 0 && Z() == 0 ? 0 : atan2(Perp(), Z()); }
    /**  Cosine of the polar angle. */
    DataType CosTheta() const { const double pTot = Mag(); return pTot == 0 ? 1 : Z() / pTot; }
    /**  The magnitude squared (rho^2 in spherical coordinate system). */
    DataType Mag2() const { return X() * X() + Y() * Y() + Z() * Z(); }
    /**  The magnitude (rho in spherical coordinate system). */
    DataType Mag() const { return std::hypot((double)Perp(), (double)Z()); }

    /**  Set phi keeping mag and theta constant. */
    void SetPhi(DataType phi)
    {
      const double perp   = Perp();
      SetX(perp * cos((double)phi));
      SetY(perp * sin((double)phi));
    }

    /**  Set theta keeping mag and phi constant. */
    void SetTheta(DataType theta)
    {
      const double ma   = Mag();
      const double ph   = Phi();
      const double ctheta = std::cos((double) theta);
      const double stheta = std::sin((double) theta);
      SetX(ma * stheta * std::cos(ph));
      SetY(ma * stheta * std::cos(ph));
      SetZ(ma * ctheta);
    }

    /**  Set magnitude keeping theta and phi constant. */
    void SetMag(DataType mag)
    {
      double factor = Mag();
      if (factor == 0) {
        B2WARNING(name() << "::SetMag: zero vector can't be stretched");
      } else {
        factor = mag / factor;
        SetX(X()*factor);
        SetY(Y()*factor);
        SetZ(Z()*factor);
      }
    }

    /**  The transverse component squared (R^2 in cylindrical coordinate system). */
    DataType Perp2() const { return X() * X() + Y() * Y(); }
    /**  The transverse component (R in cylindrical coordinate system). */
    DataType Pt() const { return Perp(); }
    /**  The transverse component (R in cylindrical coordinate system). */
    DataType Perp() const { return std::hypot((double)X(), (double)Y()); }

    /**  Set the transverse component keeping phi and z constant. */
    void SetPerp(DataType r)
    {
      const double p = Perp();
      if (p != 0.0) {
        m_coordinates[0] *= r / p;
        m_coordinates[1] *= r / p;
      }
    }

    /**  The transverse component w.r.t. given axis squared. */
    DataType Perp2(const B2Vector3<DataType>& axis)  const
    {
      const double tot = axis.Mag2();
      const double ss  = Dot(axis);
      double per = Mag2();
      if (tot > 0.0) per -= ss * ss / tot;
      if (per < 0)   per = 0;
      return per;
    }

    /**  The transverse component w.r.t. given axis. */
    DataType Pt(const B2Vector3<DataType>& axis) const { return Perp(axis); }
    /**  The transverse component w.r.t. given axis. */
    DataType Perp(const B2Vector3<DataType>& axis) const { return std::sqrt(Perp2(axis)); }
    /** returns phi in the interval [-PI,PI) */
    DataType DeltaPhi(const B2Vector3<DataType>& v) const { return Mpi_pi(Phi() - v.Phi()); }


    /** returns given angle in the interval [-PI,PI) */
    static DataType Mpi_pi(DataType angle)
    {
      if (std::isnan(angle)) {
        B2ERROR(name() << "::Mpi_pi: function called with NaN");
        return angle;
      }
      angle = std::remainder(angle, 2 * M_PI);
      //for compatibility with ROOT we flip the sign for exactly pi
      if (angle == M_PI) angle = -M_PI;
      return angle;
    }

    /** return deltaR with respect to input-vector */
    DataType DeltaR(const B2Vector3<DataType>& v) const
    {
      const double deta = Eta() - v.Eta();
      const double dphi = DeltaPhi(v);
      return std::hypot(deta, dphi);
    }

    /** return DrEtaPhi with respect to input-vector */
    DataType DrEtaPhi(const B2Vector3<DataType>& v) const
    {
      return DeltaR(v);
    }

    /** setter with mag, theta, phi */
    void SetMagThetaPhi(DataType mag, DataType theta, DataType phi)
    {
      const double amag = std::abs(mag);
      const double sinTheta = std::sin((double)theta);
      m_coordinates[0] = amag * sinTheta * std::cos((double)phi);
      m_coordinates[1] = amag * sinTheta * std::sin((double)phi);
      m_coordinates[2] = amag * std::cos((double)theta);
    }

    /**  Unit vector parallel to this. */
    B2Vector3<DataType> Unit() const
    {
      const double  tot = Mag2();
      B2Vector3<DataType> p(X(), Y(), Z());
      return tot > 0.0 ? p *= (1.0 / std::sqrt(tot)) : p;
    }

    /**  Vector orthogonal to this one. */
    B2Vector3<DataType> Orthogonal() const
    {
      const double xVal = std::abs((double)X());
      const double yVal = std::abs((double)Y());
      const double zVal = std::abs((double)Z());
      if (xVal < yVal) {
        return xVal < zVal ? B2Vector3<DataType>(0, Z(), -Y()) : B2Vector3<DataType>(Y(), -X(), 0);
      } else {
        return yVal < zVal ? B2Vector3<DataType>(-Z(), 0, X()) : B2Vector3<DataType>(Y(), -X(), 0);
      }
    }

    /**  Scalar product. */
    DataType Dot(const B2Vector3<DataType>& p) const
    {
      return X() * p.X() + Y() * p.Y() + Z() * p.Z();
    }

    /**  Cross product. */
    B2Vector3<DataType> Cross(const B2Vector3<DataType>& p) const
    {
      return B2Vector3<DataType>(Y() * p.Z() - p.Y() * Z(), Z() * p.X() - p.Z() * X(), X() * p.Y() - p.X() * Y());
    }

    /**  The angle w.r.t. another B2Vector3. */
    DataType Angle(const B2Vector3<DataType>& q) const
    {
      const double ptot2 = Mag2() * q.Mag2();
      if (ptot2 <= 0) {
        return 0.0;
      } else {
        double arg = Dot(q) / std::sqrt(ptot2);
        if (arg >  1.0) arg =  1.0;
        if (arg < -1.0) arg = -1.0;
        return std::acos(arg);
      }
    }

    /**  Returns the pseudo-rapidity, i.e. -ln(tan(theta/2)).
     *
     * for the sake of keeping compatibility to TVector3, the hardcoded values are not replaced by something more intelligent
     */
    DataType PseudoRapidity() const
    {
      const double cosTheta = CosTheta();
      if (std::abs(cosTheta) < 1) return -0.5 * std::log((1.0 - cosTheta) / (1.0 + cosTheta));
      if (Z()  == 0) return 0;
      //B2WARNING(name() << "::PseudoRapidity: transverse momentum = 0! return +/- 10e10");
      if (Z() > 0) return 10e10;
      else        return -10e10;
    }


    /** Returns the pseudo-rapidity */
    DataType Eta() const { return PseudoRapidity(); }


    /**  Rotates the B2Vector3 around the x-axis. */
    void RotateX(DataType angle)
    {
      //rotate vector around X
      const double s = std::sin((double)angle);
      const double c = std::cos((double)angle);
      const double yOld = Y();
      m_coordinates[1] = c * yOld - s * Z();
      m_coordinates[2] = s * yOld + c * Z();
    }


    /**  Rotates the B2Vector3 around the y-axis. */
    void RotateY(DataType angle)
    {
      //rotate vector around Y
      const double s = std::sin((double)angle);
      const double c = std::cos((double)angle);
      const double zOld = Z();
      m_coordinates[0] = s * zOld + c * X();
      m_coordinates[2] = c * zOld - s * X();
    }


    /**  Rotates the B2Vector3 around the z-axis. */
    void RotateZ(DataType angle)
    {
      //rotate vector around Z
      const double s = std::sin((double)angle);
      const double c = std::cos((double)angle);
      const double xOld = X();
      m_coordinates[0] = c * xOld - s * Y();
      m_coordinates[1] = s * xOld + c * Y();
    }

    /**  Rotates reference frame from Uz to newUz (unit vector). */
    void RotateUz(const B2Vector3<DataType>& NewUzVector)
    {
      // NewUzVector must be normalized !

      const double u1 = NewUzVector.X();
      const double u2 = NewUzVector.Y();
      const double u3 = NewUzVector.Z();
      double up = u1 * u1 + u2 * u2;

      if (up) {
        up = std::sqrt(up);
        DataType px = X(),  py = Y(),  pz = Z();
        m_coordinates[0] = (u1 * u3 * px - u2 * py + u1 * up * pz) / up;
        m_coordinates[1] = (u2 * u3 * px + u1 * py + u2 * up * pz) / up;
        m_coordinates[2] = (u3 * u3 * px -      px + u3 * up * pz) / up;
      } else if (u3 < 0.) {
        m_coordinates[0] = -m_coordinates[0];
        m_coordinates[2] = -m_coordinates[2];
      }
    }

    /** calculates the absolute value of the coordinates element-wise */
    void Abs()
    {
      m_coordinates[0] = std::abs(m_coordinates[0]);
      m_coordinates[1] = std::abs(m_coordinates[1]);
      m_coordinates[2] = std::abs(m_coordinates[2]);
    }

    /** calculates the square root of the absolute values of the coordinates element-wise */
    void Sqrt()
    {
      Abs();
      m_coordinates[0] = std::sqrt(m_coordinates[0]);
      m_coordinates[1] = std::sqrt(m_coordinates[1]);
      m_coordinates[2] = std::sqrt(m_coordinates[2]);
    }

    /** safe member access (with boundary check!) */
    DataType at(unsigned i) const;
    /** access variable X (= .at(0) without boundary check) */
    DataType x() const { return m_coordinates[0]; }
    /** access variable Y (= .at(1) without boundary check) */
    DataType y() const { return m_coordinates[1]; }
    /** access variable Z (= .at(2) without boundary check) */
    DataType z() const { return m_coordinates[2]; }
    /** access variable X (= .at(0) without boundary check) */
    DataType X() const { return x(); }
    /** access variable Y (= .at(1)  without boundary check) */
    DataType Y() const { return y(); }
    /** access variable Z (= .at(2) without boundary check) */
    DataType Z() const { return z(); }
    /** access variable X (= .at(0) without boundary check) */
    DataType Px() const { return x(); }
    /** access variable Y (= .at(1) without boundary check) */
    DataType Py() const { return y(); }
    /** access variable Z (= .at(2) without boundary check) */
    DataType Pz() const { return z(); }

    /** directly copies coordinates to an array of double */
    void GetXYZ(Double_t* carray) const;
    /** directly copies coordinates to an array of float */
    void GetXYZ(Float_t* carray) const;
    /** directly copies coordinates to a TVector3 */
    void GetXYZ(TVector3* tVec) const;
    /** returns a TVector3 containing the same coordinates */
    TVector3 GetTVector3() const;

    /** set X/1st-coordinate */
    void SetX(DataType x) { m_coordinates[0] = x; }
    /** set Y/2nd-coordinate */
    void SetY(DataType y) { m_coordinates[1] = y; }
    /** set Z/3rd-coordinate */
    void SetZ(DataType z) { m_coordinates[2] = z; }

    /** set all coordinates using data type */
    void SetXYZ(DataType x, DataType y, DataType z)
    {
      SetX(x); SetY(y); SetZ(z);
    }
    /** set all coordinates using a reference to TVector3 */
    void SetXYZ(const TVector3& tVec);
    /** set all coordinates using a pointer to TVector3 */
    void SetXYZ(const TVector3* tVec);

    /** Returns the name of the B2Vector. */
    static std::string name();

    /** create a string containing vector in cartesian and spherical coordinates */
    std::string PrintString(unsigned precision = 4) const
    {
      return name() + " " + PrintStringXYZ(precision) + " " + PrintStringCyl(precision);
    }

    /** create a string containing vector in cartesian coordinates */
    std::string PrintStringXYZ(unsigned precision = 4) const
    {
      std::ostringstream output;
      output  << "(x,y,z)=("
              << std::fixed << std::setprecision(precision)
              << X() << "," << Y() << "," << Z() << ")";
      return output.str();
    }

    /** create a string containing vector in spherical coordinates */
    std::string PrintStringCyl(unsigned precision = 4) const
    {
      std::ostringstream output;
      output  << "(rho, theta, phi)=("
              << std::fixed << std::setprecision(precision)
              << Mag() << "," << Theta() * 180. / M_PI << "," << Phi() * 180. / M_PI << ")";
      return output.str();
    }

    /** just for backward compatibility, should not be used with new code */
    void Print()
    {
      //print vector parameters
      Print(PrintString().c_str());
    }

  };

  /** typedef for common usage with double */
  typedef B2Vector3<double> B2Vector3D;

  /** typedef for common usage with float */
  typedef B2Vector3<float> B2Vector3F;

  /** non-memberfunction Comparison for equality with a TVector3 */
  template <typename DataType>
  Bool_t operator == (const TVector3& a, const B2Vector3<DataType>& b)
  {
    return (a.X() == b.X() && a.Y() == b.Y() && a.Z() == b.Z());
  }

  /** non-memberfunction Comparison for equality with a TVector3 */
  template < typename DataType>
  Bool_t operator != (const TVector3& a, const B2Vector3<DataType>& b)
  {
    return !(a == b);
  }

  /** non-memberfunction Scaling of 3-vectors with a real number */
  template < typename DataType>
  B2Vector3<DataType> operator * (DataType a, const B2Vector3<DataType>& p)
  {
    return B2Vector3<DataType>(a * p.X(), a * p.Y(), a * p.Z());
  }

  /** non-memberfunction for adding a TVector3 to a B2Vector3 */
  template < typename DataType>
  B2Vector3<DataType> operator + (const TVector3& a, const B2Vector3<DataType>& b)
  {
    return B2Vector3<DataType>(a.X() + b.X(), a.Y() + b.Y(), a.Z() + b.Z());
  }

  /** non-memberfunction for substracting a TVector3 from a B2Vector3 */
  template < typename DataType>
  B2Vector3<DataType> operator - (const TVector3& a, const B2Vector3<DataType>& b)
  {
    return B2Vector3<DataType>(a.X() - b.X(), a.Y() - b.Y(), a.Z() - b.Z());
  }

  /** non-memberfunction for adding a B2Vector3 to a TVector3 */
  template < typename DataType>
  B2Vector3<DataType> operator + (const B2Vector3<DataType>& a, const TVector3& b)
  {
    return B2Vector3<DataType>(a.X() + b.X(), a.Y() + b.Y(), a.Z() + b.Z());
  }

  /** non-memberfunction for substracting a B2Vector3 from a TVector3 */
  template < typename DataType>
  B2Vector3<DataType> operator - (const B2Vector3<DataType>& a, const TVector3& b)
  {
    return B2Vector3<DataType>(a.X() - b.X(), a.Y() - b.Y(), a.Z() - b.Z());
  }


  /** Assignment via B2Vector3 */
  template< typename DataType >
  B2Vector3<DataType>& B2Vector3<DataType>::operator = (const B2Vector3<DataType>& b)
  {
    m_coordinates[0] = b.X();
    m_coordinates[1] = b.Y();
    m_coordinates[2] = b.Z();
    return *this;
  }

  /** Assignment via TVector3 */
  template< typename DataType >
  B2Vector3<DataType>& B2Vector3<DataType>::operator = (const TVector3& b)
  {
    m_coordinates[0] = b.X();
    m_coordinates[1] = b.Y();
    m_coordinates[2] = b.Z();
    return *this;
  }

  /** addition */
  template< typename DataType >
  B2Vector3<DataType>& B2Vector3<DataType>::operator += (const B2Vector3<DataType>& b)
  {
    m_coordinates[0] += b.X();
    m_coordinates[1] += b.Y();
    m_coordinates[2] += b.Z();
    return *this;
  }


  /** subtraction */
  template< typename DataType >
  B2Vector3<DataType>& B2Vector3<DataType>::operator -= (const B2Vector3<DataType>& b)
  {
    m_coordinates[0] -= b.X();
    m_coordinates[1] -= b.Y();
    m_coordinates[2] -= b.Z();
    return *this;
  }

  /** multiplication with a real number */
  template< typename DataType >
  B2Vector3<DataType>& B2Vector3<DataType>::operator *= (DataType a)
  {
    m_coordinates[0] *= a;
    m_coordinates[1] *= a;
    m_coordinates[2] *= a;
    return *this;
  }

  /** set all coordinates using a reference to TVector3 */
  template< typename DataType >
  void B2Vector3<DataType>::SetXYZ(const TVector3& tVec)
  {
    m_coordinates[0] = static_cast<Double_t>(tVec.X());
    m_coordinates[1] = static_cast<Double_t>(tVec.Y());
    m_coordinates[2] = static_cast<Double_t>(tVec.Z());
  }

  /** set all coordinates using a pointer to TVector3 */
  template< typename DataType >
  void B2Vector3<DataType>::SetXYZ(const TVector3* tVec)
  {
    m_coordinates[0] = static_cast<Double_t>(tVec->X());
    m_coordinates[1] = static_cast<Double_t>(tVec->Y());
    m_coordinates[2] = static_cast<Double_t>(tVec->Z());
  }

  template< typename DataType >
  void B2Vector3<DataType>::GetXYZ(double* carray) const
  {
    carray[0] = X();
    carray[1] = Y();
    carray[2] = Z();
  }

  /** directly copies coordinates to a TVector3 */
  template< typename DataType >
  void B2Vector3<DataType>::GetXYZ(TVector3* tVec) const
  {
    tVec->SetXYZ(static_cast<Double_t>(X()),
                 static_cast<Double_t>(Y()),
                 static_cast<Double_t>(Z()));
  }


  /** returns a TVector3 containing the same coordinates */
  template< typename DataType >
  TVector3 B2Vector3<DataType>::GetTVector3() const
  {
    return
      TVector3(
        static_cast<Double_t>(X()),
        static_cast<Double_t>(Y()),
        static_cast<Double_t>(Z())
      );
  }


  /** safe member access (with boundary check!) should always be used! */
  template < typename DataType>
  DataType B2Vector3<DataType>::at(unsigned i) const
  {
    switch (i) {
      case 0:
        return B2Vector3<DataType>::m_coordinates[0];
      case 1:
        return B2Vector3<DataType>::m_coordinates[1];
      case 2:
        return B2Vector3<DataType>::m_coordinates[2];
    }
    B2FATAL(this->name() << "::access operator: given index (i=" << i << ") is out of bounds!");
    return 0.;
  }

  /** Returns a less readable (but more compatible) name of the B2Vector3.*/
  template < typename DataType>
  std::string B2Vector3<DataType>::name()
  {
    return std::string("B2Vector3<") + typeid(DataType).name() + std::string(">");
  }

} // end namespace Belle2
