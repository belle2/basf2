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

// #include <tracking/trackFindingVXD/FilterTools/TBranchLeafType.h>


#include <framework/core/FrameworkExceptions.h>
#include <framework/logging/Logger.h>

#include <TBranch.h>
#include <TTree.h>
#include <TVector3.h>
#include <string>

#include <typeinfo>
#include <cxxabi.h>

#include <algorithm>    // std::move (ranges)
#include <utility>      // std::move (objects)

#include <type_traits>

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



    /** *********************************************** EXCEPTIONS *********************************************** */


    /** this exception is thrown by the CircleFit and occurs when the track is too straight */
    BELLE2_DEFINE_EXCEPTION(outOfBounds, "B2Vector3::access operator: given index is out of bounds!");



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
    DataType operator()(unsigned i) const {
      return this->at(i);
    }


    /** safe member access (with boundary check) */
    DataType operator[](unsigned i) const {
      return this->at(i);
    }


//  /** Assignment. */
//  inline B2Vector3<DataType> & operator = (const B2Vector3<DataType> &) {
//
//  }
//
//
//  /** Comparison for equality (Geant4). */
//  inline Bool_t operator == (const B2Vector3<DataType> &) const {
//
//  }
//
//
//  /** Comparison != (Geant4). */
//  inline Bool_t operator != (const B2Vector3<DataType> &) const {
//
//  }


//  /** adding by coordinate */
//  B2Vector3<DataType> operator + (const B2Vector3<DataType> & a, const B2Vector3<DataType> & b) {
//    return B2Vector3<DataType>(a.X() + b.X(), a.Y() + b.Y(), a.Z() + b.Z());
//  }
//
//
//  /** Subtraction by coordinate */
//  B2Vector3<DataType> operator - (const B2Vector3<DataType> & a, const B2Vector3<DataType> & b) {
//    return B2Vector3<DataType>(a.X() - b.X(), a.Y() - b.Y(), a.Z() - b.Z());
//  }
//
//
//  /** scaling with real numbers */
//  B2Vector3<DataType> operator * (const B2Vector3<DataType> & p, Double_t a) {
//    return B2Vector3<DataType>(a*p.X(), a*p.Y(), a*p.Z());
//  }
//
//
//  /** ascaling with real numbers */
//  B2Vector3<DataType> operator * (Double_t a, const B2Vector3<DataType> & p) {
//    return B2Vector3<DataType>(a*p.X(), a*p.Y(), a*p.Z());
//  }


    /** TODO */
//  Double_t operator * (const B2Vector3<DataType> & a, const B2Vector3<DataType> & b) {
//    return a.Dot(b);
//  }


//  /** Addition. */
//  inline B2Vector3<DataType> & operator += (const B2Vector3<DataType> &) {
//
//  }
//
//
//  /** Subtraction. */
//  inline B2Vector3<DataType> & operator -= (const B2Vector3<DataType> &) {
//
//  }
//
//
//  /** Unary minus. */
//  inline B2Vector3<DataType> operator - () const {}
//
//
//  /** Scaling with real numbers. */
//  inline B2Vector3<DataType> & operator *= (DataType) {}



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
        std::move(
          TVector3(
            static_cast<Double_t>(X()),
            static_cast<Double_t>(Y()),
            static_cast<Double_t>(Z())
          )
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

  }; //B2Vector3 - end


  /** typedef for common usage with double */
  typedef B2Vector3<double> B2Vector3D;


  /** typedef for common usage with float */
  typedef B2Vector3<float> B2Vector3F;



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


  /** Returns the name of the B2Vector3.
   *
   */
  template < typename DataType>
  const std::string B2Vector3<DataType>::name(void) const
  {
    char* realname(NULL);
    int status(0);
    realname = abi::__cxa_demangle(typeid(*this).name(), 0, 0, &status);
    std::string name(realname);
    free(realname);
    return name ;
  }


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
}
