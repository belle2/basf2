/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *               Eugenio Paoloni                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once


// #include <framework/core/FrameworkExceptions.h>
#include <framework/logging/Logger.h>

#include <TBranch.h>
#include <TTree.h>
#include <TVector3.h>
#include <TMath.h>
#include <string>

#include <typeinfo>
// #if 0 || defined(__CINT__) || defined(__ROOTCLING__) || defined(R__DICTIONARY_FILENAME)
// #include <cxxabi.h>
// # endif

// #include <algorithm>    // std::move (ranges)
#include <utility>      // std::move (objects)


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

  template< typename DataType>
  class B2Vector3 {
  protected:

    /** contains the coordinates in given data type */
    DataType m_coordinates[3];
  public:



    /** *********************************************** CONSTRUCTORS *********************************************** */


    /** empty Constructor sets everything to 0 */
    B2Vector3(void) : m_coordinates {static_cast<DataType>(0), static_cast<DataType>(0), static_cast<DataType>(0)} {};


    /** Constructor expecting 3 coordinates */
    B2Vector3(const DataType x, const DataType y, const DataType z): m_coordinates {x, y, z} {};


    /** Constructor using a hard copy */
    B2Vector3(const DataType coordinates[3]): m_coordinates(coordinates) {};


    /** Constructor using a reference */
    B2Vector3(const DataType(& coordinates)[3]) {
      for (unsigned i = 0 ; i < 3; ++i) {
        m_coordinates[i] = coordinates[i];
      }
    };


    /** Constructor using a pointer */
    B2Vector3(const DataType(* coordinates)[3]) {
      for (unsigned i = 0 ; i < 3; ++i) {
        m_coordinates[i] = (*coordinates)[i];
      }
    };


    /** Constructor expecting a TVector3, please be careful, when DataType is not a basic type! */
    B2Vector3(const TVector3& tVec3): m_coordinates {static_cast<DataType>(tVec3.X()), static_cast<DataType>(tVec3.Y()), static_cast<DataType>(tVec3.Z())} {};


    /** Constructor expecting a pointer to a TVector3, please be careful, when DataType is not a basic type! */
    B2Vector3(const TVector3* tVec3): m_coordinates {static_cast<DataType>(tVec3->X()), static_cast<DataType>(tVec3->Y()), static_cast<DataType>(tVec3->Z())} {};


    /** Constructor expecting a B2Vector3 of same type */
    B2Vector3(const B2Vector3<DataType>& b2Vec3): m_coordinates {b2Vec3.X(), b2Vec3.Y(), b2Vec3.Z()} {};


    /** Constructor expecting a pointer to a B2Vector3 */
    B2Vector3(const B2Vector3<DataType>* b2Vec3): m_coordinates {b2Vec3->X(), b2Vec3->Y(), b2Vec3->Z()} {};


    /** Constructor expecting a B2Vector3 of different type*/
    template <typename OtherType>
    B2Vector3(const B2Vector3<OtherType>& b2Vec3): m_coordinates {static_cast<DataType>(b2Vec3.X()), static_cast<DataType>(b2Vec3.Y()), static_cast<DataType>(b2Vec3.Z())} {};


    /** Constructor expecting a pointer to a B2Vector3 of different type */
    template <typename OtherType>
    B2Vector3(const B2Vector3<OtherType>* b2Vec3): m_coordinates {static_cast<DataType>(b2Vec3->X()), static_cast<DataType>(b2Vec3->Y()), static_cast<DataType>(b2Vec3->Z())} {};



    /** *********************************************** OPERATORS *********************************************** */


    /** safe member access (with boundary check) */
    DataType operator()(unsigned i) const { return this->at(i); }


    /** safe member access (with boundary check) */
    DataType operator[](unsigned i) const { return this->at(i); }


    /** Assignment via B2Vector3 */
    inline B2Vector3<DataType>& operator = (const B2Vector3<DataType>& b) {
      m_coordinates[0] = b.X();
      m_coordinates[1] = b.Y();
      m_coordinates[2] = b.Z();
      return *this;
    }


    /** Assignment via TVector3 */
    inline B2Vector3<DataType>& operator = (const TVector3& b) {
      m_coordinates[0] = b.X();
      m_coordinates[1] = b.Y();
      m_coordinates[2] = b.Z();
      return *this;
    }


    /** type conversion in TVector3 */
//     operator TVector3() const { return std::move(TVector3(this->X(), this->Y(), this->Z())); } // C++11-version
    operator TVector3() const { return TVector3(this->X(), this->Y(), this->Z()); }



    /** Comparison for equality with a B2Vector3 */
    inline Bool_t operator == (const B2Vector3<DataType>& b) const {
      return (this->X() == b.X() && this->Y() == b.Y() && this->Z() == b.Z());
    }


    /** Comparison for equality with a TVector3 */
    inline Bool_t operator == (const TVector3& b) const {
      return (this->X() == b.X() && this->Y() == b.Y() && this->Z() == b.Z());
    }


    /** Comparison != with a B2Vector3 */
    inline Bool_t operator != (const B2Vector3<DataType>& b) const {
      return (this->X() != b.X() || this->Y() != b.Y() || this->Z() != b.Z());
    }


    /** Comparison != with a TVector3 */
    inline Bool_t operator != (const TVector3& b) const {
      return (this->X() != b.X() || this->Y() != b.Y() || this->Z() != b.Z());
    }


    /** addition */
    inline B2Vector3<DataType>& operator += (const B2Vector3<DataType>& b) {
      m_coordinates[0] += b.X();
      m_coordinates[1] += b.Y();
      m_coordinates[2] += b.Z();
      return *this;
    }


    /** subtraction */
    inline B2Vector3<DataType>& operator -= (const B2Vector3<DataType>& b) {
      m_coordinates[0] -= b.X();
      m_coordinates[1] -= b.Y();
      m_coordinates[2] -= b.Z();
      return *this;
    }


    /** unary minus */
    inline B2Vector3<DataType> operator - () const {
//    return std::move(B2Vector3<DataType>(-X(), -Y(), -Z())); // C++11-version
      return B2Vector3<DataType>(-X(), -Y(), -Z());
    }


    /** scaling with real numbers */
    inline B2Vector3<DataType>& operator *= (DataType a) {
      m_coordinates[0] *= a;
      m_coordinates[1] *= a;
      m_coordinates[2] *= a;
      return *this;
    }


    /**  Addition of 3-vectors. */
    B2Vector3<DataType> operator + (const B2Vector3<DataType>& b) const {
//    return std::move(B2Vector3<DataType>(X() + X(), Y() + Y(), Z() + b.Z())); // C++11-version
      return B2Vector3<DataType>(X() + X(), Y() + Y(), Z() + b.Z());
    }


    /**  Subtraction of 3-vectors */
    B2Vector3<DataType> operator - (const B2Vector3<DataType>& b) const {
//    return std::move(B2Vector3<DataType>(X() - b.X(), Y() - b.Y(), Z() - b.Z())); // C++11-version
      return B2Vector3<DataType>(X() - b.X(), Y() - b.Y(), Z() - b.Z());
    }


    /**  Scaling of 3-vectors with a real number */
    B2Vector3<DataType> operator * (DataType a) const {
//    return std::move(B2Vector3<DataType>(a * X(), a * Y(), a * Z())); // C++11-version
      return B2Vector3<DataType>(a * X(), a * Y(), a * Z()); // C++11-version
    }


    /**  Scalar product of 3-vectors. */
    DataType operator * (const B2Vector3<DataType>& b) const {
      return Dot(b);
    }


    /** *********************************************** MATHEMATICS *********************************************** */



    /**  The azimuth angle. returns phi from -pi to pi  */
    inline DataType Phi() const {
      return X() == 0 && Y() == 0 ? 0 : TMath::ATan2(X(), Y());
    }


    /**  The polar angle. */
    inline DataType Theta() const {
      return X() == 0 && Y() == 0 && Z() == 0 ? 0 : TMath::ATan2(Perp(), Z());
    }


    /**  Cosine of the polar angle. */
    inline DataType CosTheta() const {
      DataType pTot = Mag();
      return pTot == 0 ? 1 : Z() / pTot;
    }


    /**  The magnitude squared (rho^2 in spherical coordinate system). */
    inline DataType Mag2() const { return X() * X() + Y() * Y() + Z() * Z(); }


    /**  The magnitude (rho in spherical coordinate system). */
    inline DataType Mag() const { return TMath::Sqrt(Mag2()); }


    /**  Set phi keeping mag and theta constant. */
    inline void SetPhi(DataType phi) {
      DataType xy   = Perp();
      SetX(xy * TMath::Cos(phi));
      SetY(xy * TMath::Sin(phi));
    }


    /**  Set theta keeping mag and phi constant. */
    inline void SetTheta(DataType theta) {
      DataType ma   = Mag();
      DataType ph   = Phi();
      SetX(ma * TMath::Sin(theta)*TMath::Cos(ph));
      SetY(ma * TMath::Sin(theta)*TMath::Sin(ph));
      SetZ(ma * TMath::Cos(theta));
    }


    /**  Set magnitude keeping theta and phi constant. */
    inline void SetMag(DataType mag) {
      DataType factor = Mag();
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
    inline DataType Perp2() const { return X() * X() + Y() * Y(); }


    /**  The transverse component (R in cylindrical coordinate system). */
    inline DataType Pt() const { return Perp(); }


    /**  The transverse component (R in cylindrical coordinate system). */
    inline DataType Perp() const { return TMath::Sqrt(Perp2()); }

    /**  Set the transverse component keeping phi and z constant. */
    inline void SetPerp(DataType r) {
      DataType p = Perp();
      if (p != 0.0) {
        m_coordinates[0] *= r / p;
        m_coordinates[1] *= r / p;
      }
    }


    /**  The transverse component w.r.t. given axis squared. */
    inline DataType Perp2(const B2Vector3<DataType>& axis)  const {
      DataType tot = axis.Mag2();
      DataType ss  = Dot(axis);
      DataType per = Mag2();
      if (tot > 0.0) per -= ss * ss / tot;
      if (per < 0)   per = 0;
      return per;
    }


    /**  The transverse component w.r.t. given axis. */
    inline DataType Pt(const B2Vector3<DataType>& axis) const {
      return Perp(axis);
    }


    /**  The transverse component w.r.t. given axis. */
    inline DataType Perp(const B2Vector3<DataType>& axis) const {
      return TMath::Sqrt(Perp2(axis));
    }


    /** returns phi in the interval [-PI,PI) */
    inline DataType DeltaPhi(const B2Vector3<DataType>& v) const {
      return Mpi_pi(Phi() - v.Phi());
    }


    /** returns given angle in the interval [-PI,PI) */
    DataType Mpi_pi(DataType angle) const {
      if (TMath::IsNaN(angle)) {
        B2ERROR(name() << "::Mpi_pi: function called with NaN");
        return angle;
      }
      while (angle >= TMath::Pi()) angle -= 2.*TMath::Pi();
      while (angle < -TMath::Pi()) angle += 2.*TMath::Pi();
      return angle;
    }


    /** return deltaR with respect to input-vector */
    inline DataType DeltaR(const B2Vector3<DataType>& v) const {
      DataType deta = Eta() - v.Eta();
      DataType dphi = Mpi_pi(Phi() - v.Phi());
      return TMath::Sqrt(deta * deta + dphi * dphi);
    }


    /** return DrEtaPhi with respect to input-vector */
    inline DataType DrEtaPhi(const B2Vector3<DataType>& v) const {
      return DeltaR(v);
    }


    //setter with mag, theta, phi
    inline void SetMagThetaPhi(DataType mag, DataType theta, DataType phi) {
      DataType amag = TMath::Abs(mag);
      DataType sinTheta = TMath::Sin(theta);
      m_coordinates[0] = amag * sinTheta * TMath::Cos(phi);
      m_coordinates[1] = amag * sinTheta * TMath::Sin(phi);
      m_coordinates[2] = amag * TMath::Cos(theta);
    }


    /**  Unit vector parallel to this. */
    inline B2Vector3<DataType> Unit() const {
      DataType  tot = Mag2();
      B2Vector3<DataType> p(X(), Y(), Z());
      return tot > 0.0 ? p *= (1.0 / TMath::Sqrt(tot)) : p;
    }


    /**  Vector orthogonal to this one. */
    inline B2Vector3<DataType> Orthogonal() const {
      DataType x = X() < 0.0 ? -X() : X();
      DataType y = Y() < 0.0 ? -Y() : Y();
      DataType z = Z() < 0.0 ? -Z() : Z();
      if (x < y) {
        return x < z ? B2Vector3<DataType>(0, Z(), -Y()) : B2Vector3<DataType>(Y(), -X(), 0);
      } else {
        return y < z ? B2Vector3<DataType>(-Z(), 0, X()) : B2Vector3<DataType>(Y(), -X(), 0);
      }
    }


    /**  Scalar product. */
    inline DataType Dot(const B2Vector3<DataType>& p) const {
      return X() * p.X() + Y() * p.Y() + Z() * p.Z();
    }


    /**  Cross product. */
    inline B2Vector3<DataType> Cross(const B2Vector3<DataType>& p) const {
//    return std::move(B2Vector3<DataType>(Y() * p.Z() - p.Y() * Z(), Z() * p.X() - p.Z() * X(), X() * p.Y() - p.X() * Y())); // C++11-version
      return B2Vector3<DataType>(Y() * p.Z() - p.Y() * Z(), Z() * p.X() - p.Z() * X(), X() * p.Y() - p.X() * Y()); // C++11-version
    }


    /**  The angle w.r.t. another B2Vector3. */
    inline DataType Angle(const B2Vector3<DataType>& q) const {
      DataType ptot2 = Mag2() * q.Mag2();
      if (ptot2 <= 0) {
        return 0.0;
      } else {
        DataType arg = Dot(q) / TMath::Sqrt(ptot2);
        if (arg >  1.0) arg =  1.0;
        if (arg < -1.0) arg = -1.0;
        return TMath::ACos(arg);
      }
    }


    /**  Returns the pseudo-rapidity, i.e. -ln(tan(theta/2)).
     *
     * for the sake of keeping compatibility to TVector3, the hardcoded values are not replaced by something more intelligent
     */
    DataType PseudoRapidity() const {
      DataType cosTheta = CosTheta();
      if (cosTheta * cosTheta < 1) return -0.5 * TMath::Log((1.0 - cosTheta) / (1.0 + cosTheta));
      if (Z()  == 0) return 0;
      B2WARNING(name() << "::PseudoRapidity: transverse momentum = 0! return +/- 10e10");
      if (Z() > 0) return 10e10;
      else        return -10e10;
    }


    /** Returns the pseudo-rapidity */
    inline DataType Eta() const {
      return PseudoRapidity();
    }


    /**  Rotates the B2Vector3 around the x-axis. */
    void RotateX(DataType angle) {
      //rotate vector around X
      DataType s = TMath::Sin(angle);
      DataType c = TMath::Cos(angle);
      DataType yOld = Y();
      m_coordinates[1] = c * yOld - s * Z();
      m_coordinates[2] = s * yOld + c * Z();
    }


    /**  Rotates the B2Vector3 around the y-axis. */
    void RotateY(DataType angle) {
      //rotate vector around Y
      DataType s = TMath::Sin(angle);
      DataType c = TMath::Cos(angle);
      DataType zOld = Z();
      m_coordinates[0] = s * zOld + c * X();
      m_coordinates[2] = c * zOld - s * X();
    }


    /**  Rotates the B2Vector3 around the z-axis. */
    void RotateZ(DataType angle) {
      //rotate vector around Z
      DataType s = TMath::Sin(angle);
      DataType c = TMath::Cos(angle);
      DataType xOld = X();
      m_coordinates[0] = c * xOld - s * Y();
      m_coordinates[1] = s * xOld + c * Y();
    }


    /**  Rotates reference frame from Uz to newUz (unit vector). */
    void RotateUz(const B2Vector3<DataType>& NewUzVector) {
      // NewUzVector must be normalized !

      DataType u1 = NewUzVector.X();
      DataType u2 = NewUzVector.Y();
      DataType u3 = NewUzVector.Z();
      DataType up = u1 * u1 + u2 * u2;

      if (up) {
        up = TMath::Sqrt(up);
        DataType px = X(),  py = Y(),  pz = Z();
        m_coordinates[0] = (u1 * u3 * px - u2 * py + u1 * up * pz) / up;
        m_coordinates[1] = (u2 * u3 * px + u1 * py + u2 * up * pz) / up;
        m_coordinates[2] = (u3 * u3 * px -    px + u3 * up * pz) / up;
      } else if (u3 < 0.) { m_coordinates[0] = -m_coordinates[0]; m_coordinates[2] = -m_coordinates[2]; }      // phi=0  teta=pi
      else {};
    }


    /**  Rotates around the axis specified by another B2Vector3. */
//  void Rotate(DataType angle, const B2Vector3<DataType> & axis) {
//    //rotate vector
//    TRotation trans;
//    trans.Rotate(angle, axis);
//    operator*=(trans);
//  }


    /** *********************************************** Additional MATHEMATICS *********************************************** */
    /* functions defined here are not existing in TVector3 but enhance featureset of B2Vector3 */


    /** calculates the absolute value of the coordinates element-wise */
    inline void Abs() {
      m_coordinates[0] = std::abs(m_coordinates[0]);
      m_coordinates[1] = std::abs(m_coordinates[1]);
      m_coordinates[2] = std::abs(m_coordinates[2]);
    }


    /** calculates the square root of the absolute values of the coordinates element-wise */
    inline void Sqrt() {
      Abs();
      m_coordinates[0] = std::sqrt(m_coordinates[0]);
      m_coordinates[1] = std::sqrt(m_coordinates[1]);
      m_coordinates[2] = std::sqrt(m_coordinates[2]);
    }


    /** *********************************************** MEMBER ACCESS *********************************************** */


    /** **************************** GETTER ************************/


    /** safe member access (with boundary check!) should always be used! */
    DataType at(unsigned i) const;


    /** access variable X (= .at(0) ) */
    inline DataType x(void)  const { return m_coordinates[0]; }


    /** access variable Y (= .at(1) ) */
    inline DataType y(void)  const { return m_coordinates[1]; }


    /** access variable Z (= .at(2) ) */
    inline DataType z(void)  const { return m_coordinates[2]; }


    /** access variable X (= .at(0) ) */
    inline DataType X(void)  const { return m_coordinates[0]; }


    /** access variable Y (= .at(1) ) */
    inline DataType Y(void)  const { return m_coordinates[1]; }


    /** access variable Z (= .at(2) ) */
    inline DataType Z(void)  const { return m_coordinates[2]; }


    /** access variable X (= .at(0) ) */
    inline DataType Px(void) const { return m_coordinates[0]; }


    /** access variable Y (= .at(1) ) */
    inline DataType Py(void) const { return m_coordinates[1]; }


    /** access variable Z (= .at(2) ) */
    inline DataType Pz(void) const { return m_coordinates[2]; }


    /** directly copies coordinates to an array of double */
    inline void GetXYZ(Double_t* carray) const {
      carray[0] = static_cast<Double_t>(X());
      carray[1] = static_cast<Double_t>(Y());
      carray[2] = static_cast<Double_t>(Z());
    }


    /** directly copies coordinates to an array of float */
    inline void GetXYZ(Float_t* carray) const {
      carray[0] = static_cast<Float_t>(X());
      carray[1] = static_cast<Float_t>(Y());
      carray[2] = static_cast<Float_t>(Z());
    }


    /** directly copies coordinates to a TVector3 */
    inline void GetXYZ(TVector3* tVec) const {
      tVec->SetXYZ(static_cast<Double_t>(X()),
                   static_cast<Double_t>(Y()),
                   static_cast<Double_t>(Z()));
    }


    /** returns a TVector3 containing the same coordinates */
    inline TVector3 GetTVector3(void) const {
      return
//       std::move( // C++11-version
//           TVector3(
//             static_cast<Double_t>(X()),
//             static_cast<Double_t>(Y()),
//             static_cast<Double_t>(Z())
//           )
//         );
        TVector3(
          static_cast<Double_t>(X()),
          static_cast<Double_t>(Y()),
          static_cast<Double_t>(Z())
        );
    }


    /** **************************** SETTER ************************/


    /** set X/1st-coordinate */
    inline void SetX(DataType xx) { m_coordinates[0] = xx; }


    /** set Y/2nd-coordinate */
    inline void SetY(DataType yy) { m_coordinates[1] = yy; }


    /** set Z/3rd-coordinate */
    inline void SetZ(DataType zz) { m_coordinates[2] = zz; }


    /** set all coordinates using data type */
    inline void SetXYZ(DataType xx, DataType yy, DataType zz) {
      m_coordinates[0] = xx;
      m_coordinates[1] = yy;
      m_coordinates[2] = zz;
    }


    /** set all coordinates using a reference to TVector3 */
    inline void SetXYZ(const TVector3& tVec) {
      m_coordinates[0] = static_cast<Double_t>(tVec.X());
      m_coordinates[1] = static_cast<Double_t>(tVec.Y());
      m_coordinates[2] = static_cast<Double_t>(tVec.Z());
    }


    /** set all coordinates using a pointer to TVector3 */
    inline void SetXYZ(const TVector3* tVec) {
      m_coordinates[0] = static_cast<Double_t>(tVec->X());
      m_coordinates[1] = static_cast<Double_t>(tVec->Y());
      m_coordinates[2] = static_cast<Double_t>(tVec->Z());
    }

    /** *********************************************** MISCELLANEOUS *********************************************** */


    /** Returns the name of the B2Vector. */
    const std::string name(void) const;


    /** Creates and sets the addresses of the leaves to store the values of this B2Vector3.
     *
     * @param t  the pointer to the TTree that will contain the TBranch of this B2Vector3.
     * @param branchName the name of the TBranch that will host this B2Vector3.
     * @param variableName some extra name you can pass to identify this vector.
     *
     * The leaves will be named as the Type::variable_nEntry, where nEntry is the nth entry of the B2Vector3
     */
    void persist(TTree* t, const std::string& branchName, const std::string& variableName);


//    // Stream a B2Vector3 object
//  template<typename BufferType>
//  void Streamer(BufferType &b)
//  {
//
//    if (b.IsReading()) {
//    Version_t v = b.ReadVersion();
//    b >> m_coordinates;
//    } else {
//    b.WriteVersion(TShape::IsA());
//    b << m_coordinates;
//    }
//  }
  }; //B2Vector3 - end



  /** *********************************************** NON-MEMBER FUNCTIONS AND TYPEDEFS *********************************************** */

  /** typedef for common usage with double */
  typedef B2Vector3<double> B2Vector3D;


  /** typedef for common usage with float */
  typedef B2Vector3<float> B2Vector3F;


  /** non-memberfunction Comparison for equality with a TVector3 */
  template < typename DataType>
  inline Bool_t operator == (const TVector3& a, B2Vector3<DataType> b)
  {
    return (a.X() == b.X() && a.Y() == b.Y() && a.Z() == b.Z());
  }


  /** non-memberfunction Comparison for equality with a TVector3 */
  template < typename DataType>
  inline Bool_t operator != (const TVector3& a, B2Vector3<DataType> b)
  {
    return (a.X() != b.X() && a.Y() != b.Y() && a.Z() != b.Z());
  }



  /** non-memberfunction Scaling of 3-vectors with a real number */
  template < typename DataType>
  B2Vector3<DataType> operator * (DataType a, const B2Vector3<DataType>& p)
  {
    /*return std::move(B2Vector3<DataType>(a * p.X(), a * p.Y(), a * p.Z()));*/ // C++11-version
    return B2Vector3<DataType>(a * p.X(), a * p.Y(), a * p.Z());
  }


  /** non-memberfunction Scaling of 3-vectors with a real number */
// //   template < typename DataType>
//   B2Vector3D operator * (double a, const B2Vector3D & p) {
//  return std::move(B2Vector3D(a*p.X(), a*p.Y(), a*p.Z())); // C++11-version
//   }
//
//   B2Vector3F operator * (float a, const B2Vector3F & p) {
//  return std::move(B2Vector3F(a*p.X(), a*p.Y(), a*p.Z())); // C++11-version
//   }
//


  /** *********************************************** MEMBER FUNCTIONS TO BE INLINED *********************************************** */

  /** safe member access (with boundary check!) should always be used! */
  template < typename DataType>
  inline DataType B2Vector3<DataType>::at(unsigned i) const
  {
    switch (i) {
      case 0:
        return B2Vector3<DataType>::m_coordinates[0];
      case 1:
        return B2Vector3<DataType>::m_coordinates[1];
      case 2:
        return B2Vector3<DataType>::m_coordinates[2];
      default:
        B2FATAL(this->name() << "::access operator: given index (i=" << i << ") is out of bounds!");
        return 0.;
    }
    return 0.;
  }


//   #if  0 || defined(__CINT__) || defined(__ROOTCLING__) || defined(R__DICTIONARY_FILENAME)
//   /** Returns the name of the B2Vector3.
//    *
//    */
//   template < typename DataType>
//   const std::string B2Vector3<DataType>::name(void) const
//   {
//     char* realname(NULL);
//     int status(0);
//     realname = abi::__cxa_demangle(typeid(*this).name(), 0, 0, &status);
//     std::string name(realname);
//     free(realname);
//     return name ;
//   }
// # else
  /** Returns a less readable (but more compatible) name of the B2Vector3.
   *
   */
  template < typename DataType>
  const std::string B2Vector3<DataType>::name(void) const
  {
    return std::string("B2Vector3<") + typeid(DataType).name() + std::string(">");
  }
// #endif

  /** Creates and sets the addresses of the leaves to store the values of this B2Vector3.
   *
   * @param t  the pointer to the TTree that will contain the TBranch of this B2Vector3.
   * @param branchName the name of the TBranch that will host this B2Vector3.
   * @param variableName some extra name you can pass to identify this vector.
   *
   * The leaves will be named as the Type::variable_nEntry, where nEntry is the nth entry of the B2Vector3
   */
  template < typename DataType>
  void B2Vector3<DataType>::persist(TTree* t, const std::string& branchName, const std::string& variableName)
  {

    std::string leafList;
    leafList += B2Vector3<DataType>::name() + "::" + variableName;
    leafList += "X:";
    leafList += TBranchLeafType(B2Vector3<DataType>::X());

    leafList += B2Vector3<DataType>::name() + "::" + variableName;
    leafList += "Y:";
    leafList += TBranchLeafType(B2Vector3<DataType>::Y());

    leafList += B2Vector3<DataType>::name() + "::" + variableName;
    leafList += "Z:";
    leafList += TBranchLeafType(B2Vector3<DataType>::Z());

    TBranch* branch = new TBranch(t, branchName.c_str() , & B2Vector3<DataType>::m_coordinates, leafList.c_str());
    t->GetListOfBranches()->Add(branch);
  }

} // end namespace Belle2
